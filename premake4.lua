solution "Zest"

  targetdir "build"

  configurations { "Release", "Debug" }

  flags { "ExtraWarnings" }
  
  configuration "Release"
      flags { "Optimize" }
  configuration "Debug"
      flags { "Symbols" }
 
  configuration {}

  project "libzest"
    targetname "zest"
    --targetextension ".dylib"

    kind "SharedLib"
    language "C++"
    files { "src/*.cpp", "src/*.hpp" }

    links { "flusspferd" }

    configuration { "not windows" }
      includedirs { 
        "/usr/local/include/boost-1_37",
        "/Users/ash/code/js/mozjs_debug/include"
      }

      libdirs {
        "/Users/ash/code/js/mozjs_debug/lib"
      }
    configuration { "windows" }
      defines {"_WIN32_WINDOWS"}
      includedirs { 
        "C:/progra~1/flusspferd/include",
        "C:/spidermonkey/include",
      }

      libdirs {
        "C:/progra~1/flusspferd/lib",
      }
      links {"ws2_32", "mswsock"}

