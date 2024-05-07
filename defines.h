
/************************************************** ATTETNTION ***************************************************/
/* when CPU freq is change - required calculate new value for all delay (debounce, delay when calling spec func) */
/*						compile with "-O1" optimize, potentiometers 100 kOhm									 */
/*****************************************************************************************************************/

// for descriptors:
	#define UNUSED 0x00
	#define TOTAL_LEN_DESCR (9 + 9 + 9 + 9 + 7 + 6 + 6 + 9 + 9 + 9 + 5 + 9 + 5)

// polling keys:
	#define  DEB 300 /* freq of polling keyboard in "time of one main cycle pass", max 65535 */
	#define  DEB_BEET 25 /* debounce between 3 times polling PIN of keyboard after main debounce, max 255 */
	#define  THRESHOLD 5 /* threshold for ADC which defines step of values ADC */
