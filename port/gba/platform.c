#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <maxmod.h>

void gba_init_irq(void)
{
	irqInit();
	irqEnable(IRQ_VBLANK);
	mmSetVBlankHandler(mmFrame);
	irqSet(IRQ_VBLANK, mmVBlank);
	irqEnable(IRQ_VBLANK);
}

int platform_line_fudge_factor(void)
{
	return 18;
}

const char *platform_name(void)
{
	return "GBA";
}

const char *platform_get_lang(void)
{
	return "en";
}

void platform_more_options(void) { }

void platform_exit(void)
{
#ifdef GBA_PROFILE
	moncleanup();
#endif
}

void gba_vblank(void)
{
	VBlankIntrWait();
}
