from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("gtest/1.17.0")

    def build_requirements(self):
        self.tool_requires("cmake/4.2.3")
        self.tool_requires("doxygen/1.16.1")
