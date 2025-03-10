from conan import ConanFile
from conan.tools.cmake import cmake_layout


class ExampleRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package_multi"

    def requirements(self):
        self.requires("zlib/1.3.1")
        self.requires("cpp-httplib/0.18.0")
        self.requires("eigen/3.4.0")

    def layout(self):
        cmake_layout(self)