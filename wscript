import Options
from os import unlink, symlink
from os.path import exists
from logging import fatal

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.env.set_variant("Release")
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")
  if not conf.check(lib='yaml'):
    fatal("LibYAML not found.")

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.cxxflags = ["-Wall", "-Werror"]
  obj.target = "binding"
  obj.source = "binding.cc"
  obj.lib = ["yaml"]
