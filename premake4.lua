solution "Zest"

  targetdir "build"

  configurations { "Release" }

  flags { "ExtraWarnings" }
  
  project "libzest"
    targetname "zest"
    targetextension ".dylib"

    kind "SharedLib"
    language "C++"
    files { "src/*.cpp", "src/*.hpp" }

    includedirs { 
      "/usr/local/include/boost-1_37",
      "/Users/ash/code/js/mozjs_debug/include"
    }

    libdirs {
      "/Users/ash/code/js/mozjs_debug/lib"
    }

    links { "flusspferd" }
