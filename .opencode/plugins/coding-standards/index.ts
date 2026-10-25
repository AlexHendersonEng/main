/**
 * OpenCode plugin that exposes the personal coding standards as skills and a lookup tool.
 */
import { existsSync, readdirSync, readFileSync } from "node:fs"
import { homedir } from "node:os"
import path from "node:path"
import { fileURLToPath } from "node:url"
import { Plugin, Skill } from "@opencode/plugin"

/** Identifier used by OpenCode for this plugin. */
const PLUGIN_ID = "coding-standards"

/** Identifier of the skill that holds the shared standards and references. */
const STANDARDS_SKILL_ID = "coding-standards"

/** Language reference names that live under the standards skill. */
const LANGUAGES = ["global", "cpp", "python", "web", "markdown"] as const

/** Maps a source-file extension to the matching language reference. */
const LANGUAGE_BY_EXTENSION: Record<string, (typeof LANGUAGES)[number]> = {
  ".cpp": "cpp",
  ".cc": "cpp",
  ".cxx": "cpp",
  ".h": "cpp",
  ".hpp": "cpp",
  ".hh": "cpp",
  ".hxx": "cpp",
  ".ipp": "cpp",
  ".inl": "cpp",
  ".tpp": "cpp",
  ".ixx": "cpp",
  ".cppm": "cpp",
  ".py": "python",
  ".pyi": "python",
  ".js": "web",
  ".jsx": "web",
  ".ts": "web",
  ".tsx": "web",
  ".mjs": "web",
  ".cjs": "web",
  ".html": "web",
  ".css": "web",
  ".scss": "web",
  ".vue": "web",
  ".svelte": "web",
  ".md": "markdown",
  ".mdx": "markdown",
}

/** One skill loaded from a SKILL.md file. */
type LoadedSkill = {
  id: string
  name: string
  description?: string
  path: string
  content: string
}

/** Removes surrounding quotes from a frontmatter scalar value. */
function unquote(value: string) {
  return value.trim().replace(/^["']|["']$/g, "")
}

/** Parses the small subset of YAML used by skill frontmatter. */
function parseFrontmatter(source: string): Record<string, string> {
  const data: Record<string, string> = {}
  const lines = source.split(/\r?\n/)
  let index = 0

  while (index < lines.length) {
    const match = /^([A-Za-z0-9_-]+):\s*(.*)$/.exec(lines[index])

    if (!match) {
      index += 1

      continue
    }

    const key = match[1]
    const value = match[2]

    if (value === ">" || value === ">-" || value === "|" || value === "|-") {
      const folded = value.startsWith(">")
      const parts: string[] = []
      index += 1

      while (index < lines.length && (/^\s/.test(lines[index]) || lines[index].trim() === "")) {
        parts.push(lines[index].trim())
        index += 1
      }

      data[key] = parts.join(folded ? " " : "\n").trim()

      continue
    }

    data[key] = unquote(value)
    index += 1
  }

  return data
}

/** Splits a SKILL.md file into its frontmatter and Markdown body. */
function splitFrontmatter(source: string): { data: Record<string, string>; body: string } {
  if (!source.startsWith("---")) {
    return { data: {}, body: source }
  }

  const end = source.indexOf("\n---", 3)

  if (end === -1) {
    return { data: {}, body: source }
  }

  const frontmatter = source.slice(3, end).replace(/^\r?\n/, "")
  const body = source.slice(end + 4).replace(/^\r?\n/, "")

  return { data: parseFrontmatter(frontmatter), body: body.trim() }
}

/** Reads every skill in a skills directory, ignoring anything without a SKILL.md file. */
function readSkills(root: string): LoadedSkill[] {
  const skills: LoadedSkill[] = []

  for (const entry of readdirSync(root, { withFileTypes: true })) {
    if (!entry.isDirectory() || entry.name.startsWith(".")) {
      continue
    }

    const skillPath = path.join(root, entry.name, "SKILL.md")

    if (!existsSync(skillPath)) {
      continue
    }

    const source = readFileSync(skillPath, "utf8")
    const { data, body } = splitFrontmatter(source)
    const skill: LoadedSkill = {
      id: entry.name,
      name: data.name?.trim() || entry.name,
      description: data.description?.trim() || undefined,
      path: skillPath,
      content: body,
    }

    skills.push(skill)
  }

  return skills
}

/** Returns the directories that may hold the coding standards skills, in priority order. */
function skillRootCandidates(pluginDirectory: string, projectRoot: string, cwd: string) {
  const home = homedir()

  return [
    // Bundled beside the plugin, for installs copied outside this repository.
    path.join(pluginDirectory, "skills"),
    // The shared marketplace layout used by this repository.
    path.resolve(pluginDirectory, "../../../agent-marketplace/plugins/coding-standards/skills"),
    path.join(projectRoot, "agent-marketplace/plugins/coding-standards/skills"),
    path.join(cwd, "agent-marketplace/plugins/coding-standards/skills"),
    // Native OpenCode and compatibility skill locations.
    path.join(projectRoot, ".opencode/skills"),
    path.join(projectRoot, ".claude/skills"),
    path.join(projectRoot, ".agents/skills"),
    path.join(home, ".config/opencode/skills"),
    path.join(home, ".claude/skills"),
    path.join(home, ".agents/skills"),
  ]
}

/** Returns the first candidate directory that contains the coding standards skill. */
function findSkillRoot(pluginDirectory: string, projectRoot: string, cwd: string) {
  for (const candidate of skillRootCandidates(pluginDirectory, projectRoot, cwd)) {
    if (existsSync(path.join(candidate, STANDARDS_SKILL_ID, "SKILL.md"))) {
      return candidate
    }
  }

  return undefined
}

/** Returns the language reference name for a file, if one applies. */
function languageFor(filePath: string) {
  return LANGUAGE_BY_EXTENSION[path.extname(filePath).toLowerCase()]
}

/** Reads one reference file, returning undefined when it is missing. */
function readReference(root: string, name: string) {
  const referencePath = path.join(root, STANDARDS_SKILL_ID, "references", `${name}.md`)

  if (!existsSync(referencePath)) {
    return undefined
  }

  return readFileSync(referencePath, "utf8").trim()
}

/** Builds the tool response for one or more language references. */
function renderReferences(root: string, names: string[]) {
  const sections: string[] = [`Standards directory: ${root}`]

  for (const name of names) {
    const content = readReference(root, name)

    if (content) {
      sections.push(`## ${name}\n\n${content}`)
    }
  }

  if (sections.length === 1) {
    return `No standards reference found for ${names.join(", ")}.`
  }

  return sections.join("\n\n")
}

/** Builds the tool response for a workflow skill. */
function renderSkill(skill: LoadedSkill) {
  return [`Skill directory: ${path.dirname(skill.path)}`, `## ${skill.name}\n\n${skill.content}`].join("\n\n")
}

/** Lists the topics the lookup tool accepts. */
function availableTopics(skills: LoadedSkill[]) {
  return [...LANGUAGES, ...skills.map((skill) => skill.id)].join(", ")
}

/**
 * OpenCode plugin definition.
 *
 * Registers every skill from the coding standards bundle and adds a `coding_standards` tool that returns the
 * rules that apply to a file or a named topic.
 */
export default Plugin.define({
  id: PLUGIN_ID,
  async setup(ctx) {
    const pluginDirectory = path.dirname(fileURLToPath(import.meta.url))
    const projectRoot = ctx.location.project.canonical || ctx.location.directory
    const root = findSkillRoot(pluginDirectory, projectRoot, ctx.location.directory)

    if (!root) {
      console.warn("Coding standards skills were not found; the plugin is inactive.")

      return
    }

    const skills = readSkills(root)
    const skillByID = new Map(skills.map((skill) => [skill.id, skill]))

    await ctx.skill.transform((editor) => {
      for (const skill of skills) {
        editor.add({
          id: Skill.ID.make(skill.id),
          name: Skill.Name.make(skill.name),
          description: skill.description,
          path: Skill.Info.fields.path.make(skill.path),
          content: skill.content,
        })
      }
    })

    await ctx.tool.transform((editor) => {
      editor.add({
        name: "coding_standards",
        description:
          "Read the personal coding standards. Use before writing, editing, refactoring, documenting or reviewing " +
          "code, or when unsure which rule applies. Pass a file path to get every rule that applies to that file, " +
          "or a topic for a specific language reference or workflow skill.",
        input: {
          type: "object",
          properties: {
            filePath: {
              type: "string",
              description: "Path to a source file whose applicable standards should be returned.",
            },
            topic: {
              type: "string",
              description:
                `Language reference or workflow skill to read. One of: ${availableTopics(skills)}.`,
            },
          },
          additionalProperties: false,
        },
        async execute(input) {
          const args = input as { filePath?: string; topic?: string }

          if (args.filePath) {
            const language = languageFor(args.filePath)
            const names = language && language !== "global" ? ["global", language] : ["global"]

            return { content: renderReferences(root, names) }
          }

          if (args.topic) {
            const topic = args.topic.trim().replace(/\.md$/i, "").toLowerCase()

            if ((LANGUAGES as readonly string[]).includes(topic)) {
              return { content: renderReferences(root, [topic]) }
            }

            const skill = skillByID.get(topic)

            if (skill) {
              return { content: renderSkill(skill) }
            }

            return { content: `Unknown topic "${args.topic}". Available: ${availableTopics(skills)}.` }
          }

          return {
            content:
              "Provide a filePath or a topic. " +
              `Available topics: ${availableTopics(skills)}.`,
          }
        },
      })
    })
  },
})
