# niuhttpd
C++ web server framework

Support API, cgi/fast-cgi(Spawn-fcgi), php/php-fpm, web.py, WebSocket and MySQL and MongoDB
Please refer the code sample in api, cgi, webpy and ws direcory for the 2ndary development.
* Run release.sh to generate the intsallation files package
* run intsall.sh in installation package
* cd api and `make clean` then `make install` (it's for API code sample and MySQL)
* cd api and `make clean` then `make install MONGODB=1` (It's for MongoDB)
* cd cgi and `make install` (it's for CGI code sample)
* cd ws and `make install` (it's for WebSocket code sample)
* cd webpy and `make install` (it's for web.py, need to install Spawn-fcgi)
* /etc/init.d/niuhttpd start

##niu means 钮(niǔ).
