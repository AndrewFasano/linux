Linux kernel 5.15.11 + configs
====

This is a stock Linux 5.15.11 kernel with a helper script to build armel/mipsel/mipseb kernels that boot under PANDA. For each kernel, a PANDA OS Introspection profile will be generated


Custom files:
* `setup.sh`: takes an argument of arch to build
* `update_config.sh`: helper to run make olddefconfig, savedefconfig and update config.[arch]. Useful when switching kernel versions.
* `config.{armel,mipsel,mipseb}`: lightly customized config files
