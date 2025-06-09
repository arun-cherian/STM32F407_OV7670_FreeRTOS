/*
 * retarget.h
 *
 *  Created on: Jun 8, 2025
 *      Author: Arun
 */

#ifndef INC_RETARGET_H_
#define INC_RETARGET_H_



#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
int __io_putchar(int ch);
#else
int fputc(int ch, FILE *f);
#endif

#ifdef __cplusplus
}
#endif

#endif /* INC_RETARGET_H_ */
