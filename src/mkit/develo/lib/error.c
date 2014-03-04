#include "develo.h"

/* externs */
extern int develo_error;	/* latest error */
extern int develo_status;	/* transfer status */


/* ----
 * dv_get_err()
 * ----
 */

int
dv_get_err(void)
{
	return (develo_error);
}


/* ----
 * dv_get_errmsg()
 * ----
 */

char *
dv_get_errmsg(void)
{
	switch (develo_error) {
	case DV_OK:
		return ("OK");

	case DV_CRC_ERR:
		/* bad CRC */
		return ("CRC error");

	case DV_TIMEOUT_ERR:
		/* timeout error */
		switch (develo_status) {
		case DV_SEND:
			/* sending */
			return ("timeout error while sending a byte");

		case DV_RECV:
			/* receiving */
			return ("timeout error while receiving a byte");

		default:
			return ("timeout error");
		}

	case DV_INTERNAL_ERR:
		/* internal error */
		return ("Develo library internal error");

	default:
		return ("unknown error");
	}
}

