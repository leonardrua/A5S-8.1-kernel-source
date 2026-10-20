# include $(srctree)/drivers/misc/mediatek/Makefile.custom
#ifdef VENDOR_EDIT
#wen.luo@BSP.Kernel.Stability, 2018/12/24,  for enable KASAN, use GCC complile tool, add flag -Wno-misleading-indentation
ifeq ($(CONFIG_KASAN),y)
subdir-ccflags-y += -Wno-format
endif
#endif /*VENDOR_EDIT*/
obj-y += oppo_project_oldcdt.o
obj-y += oppo_project_newcdt.o
obj-y += oppo_project.o
ifeq ($(ODM_WT_EDIT),yes)
# fs/proc/oppoversion.o (ODM_WT_EDIT) already provides get_project(),
# is_project(), get_PCB_Version(), get_Modem_Version() and
# get_Operator_Version().  Defining them here as well would give duplicate
# symbol definitions at the vmlinux link.  The A.22/A.46 vendor kernels only
# carry the oppoversion implementation, so keep it that way.
ccflags-y += -DOPPO_PROJECT_NO_DUP_VERSIONS
endif
ifeq ($(CONFIG_MTK_SECURITY_SW_SUPPORT), y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/masp/asfv2/asf_inc
ccflags-y += -I$(srctree)/drivers/misc/mediatek/masp/asfv2/asf_export_inc
ccflags-y += -I$(srctree)/drivers/misc/mediatek/masp/asfv2/module
endif