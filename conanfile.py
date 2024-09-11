from conan import ConanFile
from conan.tools.files import copy
import os

class BxtConanFile(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps"

    def requirements(self):
        # to link to them you need to change cmake/deps.cmake
        self.requires("openssl/3.3.1")
        self.requires("boost/1.83.0")
        self.requires("date/3.0.1") # Use until LLVM libc++ gets chrono::from_stream and chrono::to_stream support 
        self.requires("fmt/11.0.2")
        self.requires("frozen/1.2.0")
        self.requires("yaml-cpp/0.8.0")
        self.requires("tomlplusplus/3.4.0")
        self.requires("jwt-cpp/0.7.0")
        self.requires("cpp-httplib/0.17.3")
        self.requires("parallel-hashmap/1.37")
        self.requires("libarchive/3.7.4")
        self.requires("drogon/1.9.0")
        self.requires("kangaru/4.3.0")
        self.requires("lmdb/0.9.32")
        self.requires("nlohmann_json/3.11.2")
        self.requires("scnlib/1.1.2")
        self.requires("cereal/1.3.2")
        self.requires("libcoro/0.12.1")
        self.requires("scope-lite/0.2.0")

    def configure(self):
        self.options["boost/*"].shared = True
        self.options["c-ares/*"].shared = True

    def generate(self):
         for dep in self.dependencies.values():
            if not dep.cpp_info.libdirs:
                continue
            copy(self, "*.so*", dep.cpp_info.libdirs[0], 
                                os.path.join(self.build_folder, "../bin/lib"))
