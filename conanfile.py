from conan import ConanFile
from conan.tools.files import copy
from conan.tools.cmake import cmake_layout
import os

class BxtConanFile(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "CMakeToolchain"
    options = {
        "testing": [True, False],
        "boost_log": [True, False],
    }
    default_options = {
        "testing": False,
        "boost_log": False,
    }
    parallel_downloads = True
    
    def requirements(self):
        # to link to them you need to change cmake/deps.cmake
        self.requires("lmdbxx/1.0.0")
        self.requires("cmake/4.0.1")
        self.requires("openssl/3.4.1")
        self.requires("boost/1.83.0")
        self.requires("spdlog/1.13.0")
        self.requires("date/3.0.3") # Use until LLVM libc++ gets chrono::from_stream and chrono::to_stream support
        self.requires("fmt/10.2.1")
        self.requires("frozen/1.2.0")
        self.requires("yaml-cpp/0.8.0")
        self.requires("tomlplusplus/3.4.0")
        self.requires("jwt-cpp/0.7.1")
        self.requires("cpp-httplib/0.19.0")
        self.requires("parallel-hashmap/1.37")
        self.requires("libarchive/3.7.9")
        self.requires("drogon/1.9.10")
        self.requires("kangaru/4.3.0")
        self.requires("lmdb/0.9.32")
        self.requires("nlohmann_json/3.12.0")
        self.requires("cereal/1.3.2")
        self.requires("libcoro/0.12.1")
        self.requires("scope-lite/0.2.0")
        
        self.requires("cli11/2.4.2")
        self.requires("di/1.3.0")
        self.requires("reflect-cpp/0.19.0")
        self.requires("sqlite3/3.49.1")
        self.requires("sqlgen/0.0.0")
        
        if self.options.testing:
            print("Testing enabled")
            self.requires("catch2/3.7.0")
            
    
    def layout(self):
        cmake_layout(self)
    
    # Define component dependencies
    boost_components = {
        # I/O streams functionality
        "iostreams": True,  
        # File system operations
        "filesystem": True,  
        "system": True,      # Required by filesystem, iostreams
        "atomic": True,
        "random": True,
        "regex": True,
        
        # Disabled components
        "context": False,
        "contract": False,
        "coroutine": False,
        "fiber": False,
        "graph": False,
        "graph_parallel": False,
        "json": False,
        "locale": False,
        "math": False,
        "mpi": False,
        "nowide": False,
        "program_options": False,
        "python": False,
        "serialization": False,
        "stacktrace": False,
        "test": False,
        "timer": False,
        "type_erasure": False,
        "url": False,
        "wave": False
    }
    
    boost_log_components = {
        "log",
        "chrono",
        "thread",
        "date_time",
        "exception",        
        "container",    
    }
    compression_options = {
        "acl": True,
        "zstd": True,
        "lzma": True,
        "bz2": True,
        "zlib": True,
        "lz4": True,
    }

    def configure(self):
        self.options["libarchive/*"].with_acl = True
        
        self.options["fmt/*"].shared = False
        self.options["fmt/*"].header_only = True
        self.options["spdlog/*"].shared = False
        self.options["spdlog/*"].header_only = True 
        self.options["trantor/*"].with_spdlog = True
        
        self.options["di/*"].with_extensions = True
                
        # Configure Boost components
        for component, enabled in self.boost_components.items():
            setattr(self.options["boost/*"], f"without_{component}", not enabled)
            
        # Configure Boost log components
        for component in self.boost_log_components:
            setattr(self.options["boost/*"], f"without_{component}", not self.options.boost_log)
            
        # Configure libarchive compression options
        for compression, enabled in self.compression_options.items():
            setattr(self.options["libarchive/*"], f"with_{compression}", enabled)
            
        self.options["drogon/*"].with_orm = False
        self.options["drogon/*"].with_boost = False
        
        
    def generate(self):
         for dep in self.dependencies.values():
            if not dep.cpp_info.libdirs:
                continue
            copy(self, "*.so*", dep.cpp_info.libdirs[0],
                                os.path.join(self.build_folder, "../bin/lib"))
