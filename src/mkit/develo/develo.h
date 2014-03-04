
/* path separator */
#if defined(DJGPP) || defined(MSDOS) || defined(WIN32)
#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_STRING "\\"
#else
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STRING "/"
#endif

/* command status */
#define DV_OK			 0
#define DV_ERR			-1
#define DV_CRC_ERR		-2
#define DV_TIMEOUT_ERR	-3
#define DV_INTERNAL_ERR	-99
#define DV_CMD  0
#define DV_SEND 1
#define DV_RECV 2
#define DV_ACK  0x06
#define DV_NAK  0x15

/* externs */
extern int develo;	/* develo box presence flag */

/* develo routines */
int   dv_init(void);
int   dv_set_bank(int page, int bank);
int   dv_get_bank(unsigned char *bank);
int   dv_set_ram(int addr, char *data, int len);
int   dv_get_ram(char *data, int addr, int len);
int   dv_set_vram(int addr, char *data, int len);
int   dv_get_vram(char *data, int addr, int len);
int   dv_set_color(int addr, char *data, int len);
int   dv_get_color(char *data, int addr, int len);
int   dv_exec(int addr, int slave);
int   dv_read_cd(char *data, int sect);
int   dv_send_cmd(unsigned char *buf);
int   dv_send_block(unsigned char *buf, int len);
int   dv_recv_block(unsigned char *buf, int len);
int   dv_wait_ack(void);
int   dv_send_byte(unsigned char c);
int   dv_recv_byte(void);
int   dv_check_user_break(void);
int   dv_get_err(void);
char *dv_get_errmsg(void);
int   dv_load_mx(char *fname, int *addr, int *bank, int disp);
void  dv_slave(int disp);
int   dv_slave_init(void);

