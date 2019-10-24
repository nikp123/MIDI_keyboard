
/************************************************** ATTETNTION ***************************************************/
/* when CPU freq is change - required calculate new value for all delay (debounce, delay when calling spec func) */
/*										compile with "-O1" optimize												 */
/*****************************************************************************************************************/

#define F_CPU 16000000L

#define  DEB 450 /* freq of polling keyboard in "time of one main cycle pass", max 65535 */
#define  DEB_BEET 25 /* debounce between 3 times polling PIN of keyboard after main debounce, max 255 */
#define  THRESHOLD 7 /* threshold for ADC which defines step of values ADC */

// IO ports:
#define LED_D0 4
#define LED_D1 5

#define BUT_B0 0
#define BUT_B1 1
#define BUT_B2 2
#define BUT_B3 3

#define BUT_C0 4
#define BUT_C1 5

#define BUT_D0 6
#define BUT_D1 7