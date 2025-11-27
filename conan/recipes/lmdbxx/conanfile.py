from conan import ConanFile
from conan.tools.files import get, copy
from conan.tools.layout import basic_layout
import os

class LmdbxxConan(ConanFile):
    name = "lmdbxx"
    version = "1.0.0"
    homepage = "https://github.com/hoytech/lmdbxx"
    description = "C++11 wrapper for LMDB"
    topics = ("lmdb", "database", "c++11", "header-only", "wrapper")
    license = "Unlicense"
    settings = "os", "compiler", "build_type", "arch"
    no_copy_source = True
    
    def requirements(self):
        self.requires("lmdb/0.9.32")
    
    def layout(self):
        basic_layout(self)
    
    def source(self):
        get(self, 
            "https://github.com/hoytech/lmdbxx/archive/6f497d1d8e1a6e0afa8ae83891d13b3fac68b62c.zip",
            strip_root=True)
    
    def package(self):
        copy(self, "*.h", src=os.path.join(self.source_folder, "include"), dst=os.path.join(self.package_folder, "include"))    
    def package_info(self):
        self.cpp_info.includedirs = ["include"]
