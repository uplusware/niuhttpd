/*
	Copyright (c) openheap, uplusware
	uplusware@gmail.com
*/

#include "session.h"

Session::Session(ServiceObjMap* srvobj, int sockfd, SSL* ssl, const char* clientip, X509* client_cert,
    BOOL https, BOOL http2, memory_cache* ch)
{
    m_srvobj = srvobj;
	m_sockfd = sockfd;
    m_ssl = ssl;
	m_clientip = clientip;
	m_https = https;
	m_http2 = http2;
	m_client_cert = client_cert;
	m_cache = ch;
    m_http_tunneling = NULL;
}

Session::~Session()
{
    if(m_http_tunneling)
        delete m_http_tunneling;
    m_http_tunneling = NULL;
}

void Session::Process()
{

	AUTH_SCHEME wwwauth_scheme = asNone;
    AUTH_SCHEME proxyauth_scheme = asNone;
	if(strcasecmp(CHttpBase::m_www_authenticate.c_str(), "basic") == 0)
	{
		wwwauth_scheme = asBasic;
	}
	else if(strcasecmp(CHttpBase::m_www_authenticate.c_str(), "digest") == 0)
	{
		wwwauth_scheme = asDigest;
	}
	
    if(strcasecmp(CHttpBase::m_proxy_authenticate.c_str(), "basic") == 0)
	{
		proxyauth_scheme = asBasic;
	}
	else if(strcasecmp(CHttpBase::m_proxy_authenticate.c_str(), "digest") == 0)
	{
		proxyauth_scheme = asDigest;
	}
    
    Http_Connection httpConn = httpKeepAlive;
    
    unsigned int connection_keep_alive_tickets = CHttpBase::m_connection_keep_alive_max;
    time_t first_connection_request_time = time(NULL);
    
    while(httpConn != httpClose)
    {
        
        
        IHttp * pProtocol;
        try
        {
            if(!m_https)
            {
                pProtocol = new CHttp(first_connection_request_time, CHttpBase::m_connection_keep_alive_timeout, connection_keep_alive_tickets, m_http_tunneling, m_srvobj, m_sockfd, CHttpBase::m_localhostname.c_str(), CHttpBase::m_httpport,
                    m_clientip.c_str(), m_client_cert, m_cache,
					CHttpBase::m_work_path.c_str(), &CHttpBase::m_default_webpages, &CHttpBase::m_ext_list, &CHttpBase::m_reverse_ext_list, CHttpBase::m_php_mode.c_str(),
                    CHttpBase::m_fpm_socktype, CHttpBase::m_fpm_sockfile.c_str(), 
                    CHttpBase::m_fpm_addr.c_str(), CHttpBase::m_fpm_port, CHttpBase::m_phpcgi_path.c_str(),
                    &CHttpBase::m_cgi_list,
                    CHttpBase::m_private_path.c_str(), wwwauth_scheme, proxyauth_scheme);
            }
            else if(m_https)
            {
                if(m_http2)
                {
                    pProtocol = new CHttp2(first_connection_request_time, CHttpBase::m_connection_keep_alive_timeout, connection_keep_alive_tickets, m_srvobj, m_sockfd, CHttpBase::m_localhostname.c_str(), CHttpBase::m_httpsport,
                        m_clientip.c_str(), m_client_cert, m_cache,
                        CHttpBase::m_work_path.c_str(), &CHttpBase::m_default_webpages, &CHttpBase::m_ext_list, &CHttpBase::m_reverse_ext_list, CHttpBase::m_php_mode.c_str(), 
                        CHttpBase::m_fpm_socktype, CHttpBase::m_fpm_sockfile.c_str(), 
                        CHttpBase::m_fpm_addr.c_str(), CHttpBase::m_fpm_port, CHttpBase::m_phpcgi_path.c_str(),
                        &CHttpBase::m_cgi_list,
                        CHttpBase::m_private_path.c_str(), wwwauth_scheme, proxyauth_scheme, m_ssl);
                }
                else
                {
                    pProtocol = new CHttp(first_connection_request_time, CHttpBase::m_connection_keep_alive_timeout, connection_keep_alive_tickets, m_http_tunneling, m_srvobj, m_sockfd, CHttpBase::m_localhostname.c_str(), CHttpBase::m_httpsport,
                        m_clientip.c_str(), m_client_cert, m_cache,
                        CHttpBase::m_work_path.c_str(), &CHttpBase::m_default_webpages, &CHttpBase::m_ext_list, &CHttpBase::m_reverse_ext_list, CHttpBase::m_php_mode.c_str(), 
                        CHttpBase::m_fpm_socktype, CHttpBase::m_fpm_sockfile.c_str(), 
                        CHttpBase::m_fpm_addr.c_str(), CHttpBase::m_fpm_port, CHttpBase::m_phpcgi_path.c_str(),
                        &CHttpBase::m_cgi_list,
                        CHttpBase::m_private_path.c_str(), wwwauth_scheme, proxyauth_scheme, m_ssl);
                }
            }
            else
            {
                throw(new string("wrong protocol type"));
            }
        }
        catch(string* e)
        {
            printf("Catch exception: %s\n", e->c_str());
            delete e;
            shutdown(m_sockfd, SHUT_RDWR);
            m_sockfd = -1;
            return;
        }
        int tmp1 = time(NULL);
                    
        httpConn = pProtocol->Processing();
        if(!m_http2)
        {
			m_http_tunneling  = pProtocol->GetHttpTunneling();
        }
        delete pProtocol;
        
        if(connection_keep_alive_tickets > 0)
        {
            connection_keep_alive_tickets--;
        }
        
        if(connection_keep_alive_tickets == 0 || (time(NULL) - first_connection_request_time) > CHttpBase::m_connection_keep_alive_timeout)
        {
            httpConn = httpClose;
        }
        
    }
    
    //printf("out of session\n");
}

