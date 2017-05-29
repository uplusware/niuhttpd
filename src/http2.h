/*
	Copyright (c) openheap, uplusware
	uplusware@gmail.com
*/

#ifndef _HTTP2_H_
#define _HTTP2_H_
#include <pthread.h>
#include "http.h"
#include "http2comm.h"
#include "http2stream.h"
#include "hpack.h"
#include <queue>
#include <utility>
#include <map>
using namespace std;
/*
          +-------+-----------------------------+---------------+
          | Index | Header Name                 | Header Value  |
          +-------+-----------------------------+---------------+
          | 1     | :authority                  |               |
          | 2     | :method                     | GET           |
          | 3     | :method                     | POST          |
          | 4     | :path                       | /             |
          | 5     | :path                       | /index.html   |
          | 6     | :scheme                     | http          |
          | 7     | :scheme                     | https         |
          | 8     | :status                     | 200           |
          | 9     | :status                     | 204           |
          | 10    | :status                     | 206           |
          | 11    | :status                     | 304           |
          | 12    | :status                     | 400           |
          | 13    | :status                     | 404           |
          | 14    | :status                     | 500           |
          | 15    | accept-charset              |               |
          | 16    | accept-encoding             | gzip, deflate |
          | 17    | accept-language             |               |
          | 18    | accept-ranges               |               |
          | 19    | accept                      |               |
          | 20    | access-control-allow-origin |               |
          | 21    | age                         |               |
          | 22    | allow                       |               |
          | 23    | authorization               |               |
          | 24    | cache-control               |               |
          | 25    | content-disposition         |               |
          | 26    | content-encoding            |               |
          | 27    | content-language            |               |
          | 28    | content-length              |               |
          | 29    | content-location            |               |
          | 30    | content-range               |               |
          | 31    | content-type                |               |
          | 32    | cookie                      |               |
          | 33    | date                        |               |
          | 34    | etag                        |               |
          | 35    | expect                      |               |
          | 36    | expires                     |               |
          | 37    | from                        |               |
          | 38    | host                        |               |
          | 39    | if-match                    |               |
          | 40    | if-modified-since           |               |
          | 41    | if-none-match               |               |
          | 42    | if-range                    |               |
          | 43    | if-unmodified-since         |               |
          | 44    | last-modified               |               |
          | 45    | link                        |               |
          | 46    | location                    |               |
          | 47    | max-forwards                |               |
          | 48    | proxy-authenticate          |               |
          | 49    | proxy-authorization         |               |
          | 50    | range                       |               |
          | 51    | referer                     |               |
          | 52    | refresh                     |               |
          | 53    | retry-after                 |               |
          | 54    | server                      |               |
          | 55    | set-cookie                  |               |
          | 56    | strict-transport-security   |               |
          | 57    | transfer-encoding           |               |
          | 58    | user-agent                  |               |
          | 59    | vary                        |               |
          | 60    | via                         |               |
          | 61    | www-authenticate            |               |
          +-------+-----------------------------+---------------+
*/

#define HTTP2_PREFACE_LEN 24

enum http2_msg_type
{
    http2_msg_quit = 0,
    http2_msg_data
};

typedef struct
{
    http2_msg_type msg_type;
    HTTP2_Frame* frame;
    char* payload;
} http2_msg;

class CHttp;
class http2_stream;
class CHttp2 : public IHttp
{
public:
	CHttp2(ServiceObjMap* srvobj, int sockfd, const char* servername, unsigned short serverport,
	    const char* clientip, X509* client_cert, memory_cache* ch,
		const char* work_path, vector<string>* default_webpages, vector<stExtension>* ext_list, const char* php_mode, 
        cgi_socket_t fpm_socktype, const char* fpm_sockfile, 
        const char* fpm_addr, unsigned short fpm_port, const char* phpcgi_path,
        map<string, cgi_cfg_t>* cgi_list,
        const char* private_path, AUTH_SCHEME wwwauth_scheme = asNone,
		SSL* ssl = NULL);
        
	virtual ~CHttp2();
	
	int ProtRecv();
    virtual Http_Connection Processing();
    virtual int HttpSend(const char* buf, int len);
    virtual int HttpRecv(char* buf, int len);
    
    void PushPromise(uint_32 stream_ind, const char * path);
    
    void TransHttp2ParseHttp1Header(uint_32 stream_ind, hpack* hdr);
    
    int TransHttp1SendHttp2Header(uint_32 stream_ind, const char* buf, int len, uint_8 frame_type = HTTP2_FRAME_TYPE_HEADERS, uint_32 promised_stream_ind = 0);
    int TransHttp1SendHttp2Content(uint_32 stream_ind, const char* buf, uint_32 len);
    
    int SendHttp2EmptyContent(uint_32 stream_ind, uint_8 flags = HTTP2_FRAME_FLAG_END_STREAM);
    void SendHttp2PushPromiseRequest(uint_32 stream_ind);
    void SendHttp2PushPromiseResponse();
    
    void send_setting_ack(uint_32 stream_ind);
    void send_window_update(uint_32 stream_ind, uint_32 increament_window_size);
    void send_goaway(uint_32 last_stream_ind, uint_32 error_code);
    void send_push_promise_request(uint_32 stream_ind);
    void send_rst_stream(uint_32 stream_ind);
    void send_initial_window_size(uint_32 window_size);
    
    http2_stream* create_stream_instance(uint_32 stream_ind);
    
    http2_stream* get_stream_instance(uint_32 stream_ind);
    
    uint_32  get_initial_local_window_size();
    
private:
    ServiceObjMap * m_srvobj;
    int m_sockfd;
    string m_servername;
    unsigned short m_serverport;
    string m_clientip;
    X509* m_client_cert;
    memory_cache* m_ch;
    string m_work_path;
    vector<stExtension>* m_ext_list;
    vector<string>* m_default_webpages;
    string m_php_mode;
    cgi_socket_t m_fpm_socktype;
    string m_fpm_sockfile;
    string m_fpm_addr;
    unsigned short m_fpm_port;
    string m_phpcgi_path;
    
    map<string, cgi_cfg_t>* m_cgi_list;
    
    string m_private_path;
    AUTH_SCHEME m_wwwauth_scheme;
    SSL* m_ssl;

    linesock* m_lsockfd;
	linessl * m_lssl;
    
    map<uint_32, http2_stream*> m_stream_list;
    
	char m_preface[HTTP2_PREFACE_LEN + 1];
	void init_header_table();
	map<int, pair<string, string> > m_header_static_table;
    map<int, pair<string, string> > m_header_dynamic_table;
    
    //Setting
    uint_32 m_header_table_size;
    BOOL m_enable_push;
    uint_32 m_max_concurrent_streams;
    
    uint_32 m_initial_local_window_size;
    uint_32 m_initial_peer_window_size;
    
    uint_32 m_local_window_size;    
    uint_32 m_peer_window_size;
    
    uint_32 m_max_frame_size;
    uint_32 m_max_header_list_size;
    
    BOOL m_pushed_request;
    BOOL m_pushed_response;
};

#endif /* _HTTP2_H_ */