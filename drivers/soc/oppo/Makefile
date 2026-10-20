#ifdef VENDOR_EDIT
# Bin.Li@EXP.BSP.bootloader.bootflow, 2017/05/15, Add for oppo device info and oppo project
#obj-y += oppo_custom/
#endif /* VENDOR_EDIT */
# ifdef VENDOR_EDIT 
#ifdef VENDOR_EDIT
# Bin.Li@EXP.BSP.bootloader.bootflow, 2017/05/15, Add for oppo device info and oppo project
ifeq ($(ODM_WT_EDIT),yes)
# fs/proc/oppoversion.o (ODM_WT_EDIT) already provides get_project(),
# is_project(), get_PCB_Version(), get_Modem_Version() and
# get_Operator_Version().  Building oppo_project/ as well would define the
# same symbols twice and fail the vmlinux link.  The A.22/A.46 vendor
# kernels only carry the oppoversion implementation, so keep it that way.
else
obj-$(CONFIG_OPPO_COMMON_SOFT)  +=  oppo_project/
endif
obj-$(CONFIG_OPPO_DEVICE_IFNO)  +=  oppo_devinfo/
obj-$(CONFIG_OPPO_HEALTHINFO)     +=  oppo_healthinfo/
#obj-$(CONFIG_OPPO_ALS_CALI)   +=  oppo_als_cali_value/

obj-y += oppo_custom/
#obj-$(CONFIG_OPPO_HYPNUS)       +=  oppo_hypnus/

#Liang.Zhang@TECH.Storage.Stability.OPPO_SHUTDOWN_DETECT, 2019/04/28, Add for shutdown detect
obj-$(CONFIG_OPPO_SHUTDOWN_DETECT)  +=  oppo_shutdown_detect/
#endif /* VENDOR_EDIT */
#ifdef VENDOR_EDIT
#Kun.Hu@TECH.BSP.Stability.PHOENIX_PROJECT 2019/06/15, Add for PHOENIX_PROJECT
obj-$(CONFIG_OPPO_PHOENIX)     +=  oppo_phoenix/
#endif /* VENDOR_EDIT */

