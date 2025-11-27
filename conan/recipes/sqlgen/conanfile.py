from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy, get, replace_in_file
from conan.tools.system import package_manager
import os

required_conan_version = ">=1.53.0"

class SqlgenConan(ConanFile):
    name = "sqlgen"
    version = "0.0.0"
    description = "ORM and SQL query generator for C++20"
    license = "MIT"  # Update based on actual license
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://github.com/getml/sqlgen"
    topics = ("sql", "orm", "c++20")
    package_type = "library"
    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "postgres": [True, False],
        "sqlite3": [True, False],
        "tests": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "postgres": False,
        "sqlite3": True,
        "tests": False,
    }
    
    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")
    
    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")
    
    def layout(self):
        cmake_layout(self, src_folder="src")
    
    def requirements(self):
        self.requires("reflect-cpp/0.18.0") 
        
        if self.options.postgres:
            self.requires("libpq/16.4")
        
        if self.options.sqlite3:
            self.requires("sqlite3/3.49.1")
            
        if self.options.tests:
            self.test_requires("gtest/1.14.0")
    
    def validate(self):
        check_min_cppstd(self, "20")
    
    def source(self):
        get(self, 
            "https://github.com/getml/sqlgen/archive/e4b821138a9a498059838ab9c46a4e68fd983c8f.zip",
            destination=self.source_folder,
            strip_root=True)
        
        # Replace vcpkg SQLite3 references with standard CMake ones
        self._patch_cmake_files()
    
    def _patch_cmake_files(self):
        cmake_file = os.path.join(self.source_folder, "CMakeLists.txt")
        
        # Replace find_package call
        replace_in_file(self, cmake_file,
                       "find_package(unofficial-sqlite3 CONFIG REQUIRED)",
                       "find_package(SQLite3 REQUIRED)")
        
        # Replace target check
        replace_in_file(self, cmake_file,
                       "if (NOT TARGET unofficial-sqlite3)",
                       "if (NOT TARGET SQLite3)")
        
        # Replace target link
        replace_in_file(self, cmake_file,
                       "target_link_libraries(sqlgen PUBLIC unofficial::sqlite3::sqlite3)",
                       "target_link_libraries(sqlgen PUBLIC SQLite::SQLite3)")
    
    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        
        tc = CMakeToolchain(self, generator="Ninja")
        # Disable vcpkg - important!
        tc.cache_variables["SQLGEN_USE_VCPKG"] = False
        tc.cache_variables["CMAKE_CXX_STANDARD"] = "20"
        tc.cache_variables["SQLGEN_BUILD_SHARED"] = self.options.shared
        
        # Configure optional features based on CMakeLists.txt
        tc.cache_variables["SQLGEN_POSTGRES"] = self.options.postgres
        tc.cache_variables["SQLGEN_SQLITE3"] = self.options.sqlite3
        tc.cache_variables["SQLGEN_BUILD_TESTS"] = self.options.tests
        
        tc.generate()
    
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        
        if self.options.tests:
            cmake.test()
    
    def package(self):
        copy(self, "LICENSE*", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        cmake = CMake(self)
        cmake.install()
    
    def package_info(self):
        self.cpp_info.libs = ["sqlgen"]
        self.cpp_info.set_property("cmake_file_name", "sqlgen")
        self.cpp_info.set_property("cmake_target_name", "sqlgen::sqlgen")
        
        # Require C++20
        self.cpp_info.cppstd = "20"
        
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs.append("m")
        
        if self.options.postgres:
            self.cpp_info.defines.append("SQLGEN_HAS_POSTGRES=1")
        
        if self.options.sqlite3:
            self.cpp_info.defines.append("SQLGEN_HAS_SQLITE3=1")
