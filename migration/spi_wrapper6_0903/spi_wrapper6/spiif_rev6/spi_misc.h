
#ifndef	SUSIE_SPI_MISC_H
#define	SUSIE_SPI_MISC_H

/*
 *	Susie Plugin Interface 補助ファイル
 *  					by kana
 *
 *  Report and Suggestion to
 *    NiftyServe PXB13767
 *    Email: inokuchi@mxc.meshnet.or.jp
 */
/*
 * History:
 *  97/01/17:
 *    初期作成
 */

#ifdef __cplusplus
extern "C" {
#endif


/* データソース定義 */
typedef enum {
  SUSIE_IF_SOURCE_FILE= 0,
  SUSIE_IF_SOURCE_MEMORY,
} SUSIE_IF_SOURCE_TYPE;

#define	SUSIE_IF_INTERNAL_BUFFER_SIZE	(2048)

typedef enum {
	SUSIE_IF_ERROR_NOT_IMPLEMENTED= -1,
	SUSIE_IF_ERROR_SUCCESS= 0,
	SUSIE_IF_ERROR_CANCEL_EXPAND= 1,
	SUSIE_IF_ERROR_UNKNOWN_FORMAT= 2,
	SUSIE_IF_ERROR_BROKEN_DATA= 3,
	SUSIE_IF_ERROR_ALLOCATE_MEMORY= 4,
	SUSIE_IF_ERROR_MEMORY= 5,
	SUSIE_IF_ERROR_FILE_READ= 6,
	SUSIE_IF_ERROR_RESERVED= 7,
	SUSIE_IF_ERROR_INTERNAL= 8,
	SUSIE_IF_ERROR_MAX
} SUSIE_IF_ERROR_CODE;



#ifdef __cplusplus
}
#endif


#endif  /* SUSIE_SPI_MISC_H */
