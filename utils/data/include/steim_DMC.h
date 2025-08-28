#define UBYTE unsigned char
#define WORD  short
#define UWORD unsigned short
#define LONG  gint32
#define ULONG guint32

typedef struct _BTIME {
    UWORD year ;	     /* e.g. 1991 */
    UWORD day ; 	     /* 1..366 */
    UBYTE hours ;	     /* 0..23 */
    UBYTE minutes ;	     /* 0..59 */
    UBYTE seconds ;	     /* 0..59, 60 for leap */
    UBYTE alignment_1 ;
    UWORD frac_secs ;		/* 0.0001 seconds, 0..9999 */
    } BTIME ;

typedef struct _DATA_TYPE {
    char   SequenceNumber[6] ;
    char   Data_header_indicator ;
    char   Reserved_bytes_A ;
} DATA_TYPE;

typedef struct _DATA_HEADER {
    char   SequenceNumber[6] ;
    char   Data_header_indicator ;
    char   Reserved_bytes_A ;
    char   Station_identifier_code[5] ;
    char   Location_identifier[2] ;
    char   Channel_identifier[3] ;
    char   Network_Code[2] ;
    BTIME  Record_start_time ;
    UWORD  Number_of_samples ;
    WORD   Sample_rate_factor ;
    WORD   Sample_rate_multiplier ;
    UBYTE  Activity_flags ;
    UBYTE  IO_flags ;
    UBYTE  Data_quality_flags ;
    UBYTE  Number_of_blockettes_follow ;
    LONG   Time_correction ;
    UWORD  Beginning_of_data ;
    UWORD  First_blockette ;
    } DATA_HEADER ;

struct			data_blk_hdr			/* data blockette header */
{
	unsigned	short int type;			/* blockette type */
	unsigned	short int next_blk_byte;	/* start of next blockette byte number */
};

typedef struct			data_blk_100			/* blockette 100 */
{
	struct		data_blk_hdr hdr;		/* blockette header */
	float		sample_rate;			/* actual sample rate */
	unsigned	char flags;			/* flags */
	unsigned	char reserved[3];		/* reserved bytes */
      } BLK_100;

typedef struct  _BLK_1000  {  
        UWORD           blockette_type;
        UWORD           next_blockette_byte; 
        UBYTE           encoding_format; 
        UBYTE           word_order; 
        UBYTE           data_rec_length; 
        UBYTE           reserved; 
    }  BLK_1000;

#define _HUGE_
#define GM_ALLOC(NUM, SIZE) malloc((NUM)*(SIZE))
#define GM_FREE free

#ifdef ANSI_EXTENSIONS
long Steim_comp(long _HUGE_ * p_dbuf,
		DATA_HEADER * p_fsdh,
		ULONG Number_of_samples,
		WORD data_rec_length,
		LONG _HUGE_ * p_seed_data_records,
		ULONG num_cont_blk,
		ULONG num_cont_blk_bytes,
		char *cont_blk,
		ULONG num_once_blk,
		ULONG num_once_blk_bytes,
		char *once_blk) ;
#endif
