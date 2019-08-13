#ifndef	CONVERT_THREAD_H
#define	CONVERT_THREAD_H

typedef struct {
	SpiInputStream m_in;
	int m_result;
	unsigned long m_data32;
} convert_thread_param_t;

class convert_thread_t {
public:
	convert_thread_t();
	virtual ~convert_thread_t();

	void set_plugin_manager(SpiFilesManage *manager);
	int set_parameter(const char *targetdir, HWND hWnd, UINT wMsg, convert_thread_param_t *convert_files, int num);
	int start(void);
	int cancel(void);
	int is_running(void);

protected:
	/* スレッド関数 */
	static unsigned int __stdcall threadex_func(void *param);
	static int PASCAL callback_func(int num, int denom, long l_data);

private:
	/* Plug-in集合 */
	SpiFilesManage *m_manager;

	/* 出力先ディレクトリ */
	char *m_target_dir;

	/* パラメータ */
	HWND m_notify_hWnd;
	UINT m_notify_wMsg;

	convert_thread_param_t *m_convert_files;
	int m_convert_file_num;
	int m_convert_progress;

	/* キャンセルフラグ */
	int m_do_cancel;

	/* スレッドハンドル */
	HANDLE m_hThread;
};


#endif	/* CONVERT_THREAD_H */
