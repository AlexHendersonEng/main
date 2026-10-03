use assert_cmd::Command;
use predicates::prelude::*;
use std::fs;
use tempfile::TempDir;

fn findr() -> Command {
    Command::cargo_bin("findr").unwrap()
}

fn fixture() -> TempDir {
    let d = TempDir::new().unwrap();
    let p = d.path();
    fs::create_dir_all(p.join("src/inner")).unwrap();
    fs::create_dir_all(p.join(".hidden")).unwrap();
    fs::write(p.join("src/main.rs"), "fn main() {}\nlet foo = 1;\n").unwrap();
    fs::write(p.join("src/inner/lib.rs"), "pub fn foo() {}\n").unwrap();
    fs::write(p.join("notes.txt"), "Hello World\n").unwrap();
    fs::write(p.join(".hidden/secret.txt"), "foo\n").unwrap();
    fs::write(p.join("bin.dat"), [b'f', b'o', b'o', 0, 1]).unwrap();
    d
}

#[test]
fn files_matches_basenames_only() {
    let d = fixture();
    findr()
        .args(["files", r"\.rs$"])
        .arg(d.path())
        .assert()
        .success()
        .stdout(predicate::str::contains("main.rs").and(predicate::str::contains("lib.rs")))
        .stdout(predicate::str::contains("notes.txt").not());
}

#[test]
fn dirs_matches_directories() {
    let d = fixture();
    findr()
        .args(["dirs", "^inner$"])
        .arg(d.path())
        .assert()
        .success()
        .stdout(predicate::str::contains("inner"));
}

#[test]
fn no_match_exits_1() {
    let d = fixture();
    findr()
        .args(["files", "zzz"])
        .arg(d.path())
        .assert()
        .code(1);
}

#[test]
fn invalid_regex_exits_2() {
    let d = fixture();
    findr()
        .args(["files", "("])
        .arg(d.path())
        .assert()
        .code(2)
        .stderr(predicate::str::contains("invalid pattern"));
}

#[test]
fn hidden_skipped_unless_requested() {
    let d = fixture();
    findr()
        .args(["files", "secret"])
        .arg(d.path())
        .assert()
        .code(1);
    findr()
        .args(["files", "secret", "--hidden"])
        .arg(d.path())
        .assert()
        .success();
}

#[test]
fn ignore_case_and_fixed_strings() {
    let d = fixture();
    findr()
        .args(["text", "hello", "--color", "never"])
        .arg(d.path())
        .assert()
        .code(1);
    findr()
        .args(["text", "-i", "hello", "--color", "never"])
        .arg(d.path())
        .assert()
        .success()
        .stdout(
            predicate::str::contains("notes.txt")
                .and(predicate::str::contains(":1:1: Hello World")),
        );
    findr()
        .args(["text", "-F", "fn main(", "--color", "never"])
        .arg(d.path())
        .assert()
        .success();
}

#[test]
fn max_depth_limits_walk() {
    let d = fixture();
    findr()
        .args(["files", "lib", "-d", "2"])
        .arg(d.path())
        .assert()
        .code(1);
    findr()
        .args(["files", "lib", "-d", "3"])
        .arg(d.path())
        .assert()
        .success();
}

#[test]
fn glob_filters_files() {
    let d = fixture();
    findr()
        .args(["text", "foo", "-g", "*.rs", "--color", "never"])
        .arg(d.path())
        .assert()
        .success()
        .stdout(predicate::str::contains(".rs:").and(predicate::str::contains("notes.txt").not()));
}

#[test]
fn text_reports_line_and_column_and_skips_binary() {
    let d = fixture();
    findr()
        .args(["text", "foo", "--color", "never"])
        .arg(d.path())
        .assert()
        .success()
        .stdout(predicate::str::contains(":2:5: let foo = 1;"))
        .stdout(predicate::str::contains("bin.dat").not());
}

#[test]
fn replace_is_dry_run_by_default() {
    let d = fixture();
    findr()
        .args(["replace", "foo", "bar", "--color", "never"])
        .arg(d.path())
        .assert()
        .success()
        .stdout(
            predicate::str::contains("-let foo = 1;")
                .and(predicate::str::contains("+let bar = 1;")),
        );
    assert_eq!(
        fs::read_to_string(d.path().join("src/main.rs")).unwrap(),
        "fn main() {}\nlet foo = 1;\n"
    );
}

#[test]
fn replace_write_applies_and_supports_captures() {
    let d = fixture();
    findr()
        .args(["replace", r"let (\w+) = (\d)", "let $2 = $1", "--write"])
        .arg(d.path())
        .assert()
        .success();
    assert_eq!(
        fs::read_to_string(d.path().join("src/main.rs")).unwrap(),
        "fn main() {}\nlet 1 = foo;\n"
    );
    // Binary files are never modified.
    findr()
        .args(["replace", "foo", "bar", "--write"])
        .arg(d.path())
        .assert()
        .success();
    assert_eq!(
        fs::read(d.path().join("bin.dat")).unwrap(),
        vec![b'f', b'o', b'o', 0, 1]
    );
}

#[test]
fn replace_no_match_exits_1() {
    let d = fixture();
    findr()
        .args(["replace", "zzz", "y"])
        .arg(d.path())
        .assert()
        .code(1);
}
