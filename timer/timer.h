#ifndef PMA_TIMER_H
#define PMA_TIMER_H
/*!
 * the definition of callback function used in the timer modules
 * \param args the arguments used in the callback function
 *			  u can define the argument's structure by urself. it's a pointer,
 *           so you can pass any kind of data if only you package them well.
 *           but it is better not contain any other pointer in your package.
 * \return the return result
 */
typedef void *(*timer_callback)(void *args);

/*!
 * set a new timer in ther timer list.
 * \param time_val the interval of the timeout.
 * \param callback the callback function of the timer.
 * \param args the argument of the callback function.
 * \param len the length of you argument
 * \return if success, the timer id of the new timer will be returned.
 *		  if failed, it will return -1.
 */
extern u32 set_timer(int time_val, timer_callback callback, void *args, u8 len);

/*!
 * update the appointed timer, so the timer can restart.
 * use it when your expected event happen in the arrange of expected time.
 * \param t_id the timer id which you want it to restart time
 * \return if success, it's same as the argument
 *		  if failed, it will return -1.
 *		  it will return 1 if there is no such timer
 */
extern u32 update_timer(u32 t_id);

/*!
 * modify the appinted timer, so the timer's timeout(time) can be changed.
 * \param t_id the timer id which you want to change its timeout(time).
 * \return if success, it's same as the argument
 *		  if failed, it will return -1.
 *		  it will return 1 if there is no such timer
 */
extern u32 modify_timer(u32 t_id, u32 timeout);

/*!
 * delete the appointed timer from the timer list
 * \param t_id the timer id which you want to delete
 * \return if scuccess, 0 is returned.
 *		  if failed, -1 is returned..
 */
extern u32 unset_timer(u32 t_id);

/*! clear all the timer in ther timer list
 * \param void no need argument
 * \return 0 always success
 */
extern u32 clear_timer(void);

/*! initialize the timer
 * \param void no need argument
 * \return no return
 */
extern int timer_init(void);

#endif //PMA_TIMER_HEADER_H

