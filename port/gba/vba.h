#ifndef VBA_H
#define VBA_H

/**
 * Records a call graph entry. R12 should contain LR of previous caller.
 */
extern void mcount(void);
/**
 * Starts profiling on the given address range (low ... high)
 */
extern void monstartup(unsigned int low, unsigned int high);
/**
 * Controls profiling.
 *
 * mode = 0 stops profiling, mode != 0 starts it
 */
extern void moncontrol(int mode);
/**
 * Clean up and stop profiling. gmon.out will be written
 */
extern void moncleanup(void);
/**
 * Logs a message message to VBA console (or GDB console if using GDB).
 */
extern void vbalog(const char *msg);

#endif
