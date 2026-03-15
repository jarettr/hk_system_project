cmake_minimum_required(VERSION 3.10)
project(hk_system C)

DeclareCAmkESComponent(Dispatcher SOURCES components/Dispatcher/dispatcher.c)
DeclareCAmkESComponent(Worker SOURCES components/Worker/worker.c components/Worker/posix_layer.c)
DeclareCAmkESComponent(Repository SOURCES components/Repository/repository.c)
DeclareCAmkESComponent(TPM_Device SOURCES components/TPM_Device/tpm.c)
DeclareCAmkESComponent(Generator SOURCES components/Generator/generator.c)
DeclareCAmkESComponent(Scheduler SOURCES components/Scheduler/scheduler.c)
DeclareCAmkESComponent(Verifier SOURCES components/Verifier/verifier.c)
DeclareCAmkESComponent(AdminConsole SOURCES components/AdminConsole/admin.c)
DeclareCAmkESComponent(VirtualFileSystem SOURCES components/VFS/vfs_server.c)

DeclareCAmkESRootserver(hk_system.camkes)
