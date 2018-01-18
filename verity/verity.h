#ifndef VERITY_TOOL_H
#define VERITY_TOOL_H

/*
 * Return codes:
 *
 *    =0 success
 *    >0 no changes needed
 *    <0 error
 */
int toggle_verity(bool enable);

#endif
