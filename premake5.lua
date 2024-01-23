-- premake5.lua
workspace "SimpleRayTracer"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "SimpleRayTracer"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "SimpleRayTracer"