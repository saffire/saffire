FastCGI
-------------
Saffire can be run as a FastCGI process. In order to make this function, two things must be done:
   * Configure and start saffire as a fastcgi daemon
   * Setup a webserver that will connect to saffire fastcgi daemon.


saffire fastcgi daemon
----------------------
Saffire has an fastcgi server built-in. Make sure you have the correct configuration in place before starting the daemon:

     # /etc/saffire/saffire.ini
     [fastcgi]
     pid.path = ~/.saffire.pid
     log.path = /var/log/saffire/fastcgi.log
     log.level = notice
     daemonize = true
     spawn_children = 5
     user = www-data
     group = www-data
    
     #listen = 0.0.0.0:8123
     listen = /var/run/saffire.socket
     listen.socket.user = nobody
     listen.socket.group = nogroup
     listen.socket.mode = 0666
     
This should be enough to get the saffire daemon running. Note there is not yet any control available for the daemon, so you can daemonize it, but you have to kill it manually.

To start the daemon:

     saffire fastcgi 
     
    
Nginx
-----
The easiest way to setup a webserver is to use Nginx. You need to create a new server configuration, which looks something like this:

    server {
        listen   80;     # Listen to connections on port 80

        root /usr/share/nginx/www;
        index index.html index.htm index.sf;
        
        # Make site accessible from http://localhost/
        server_name localhost;

        # Enable fastCGI whenever we try and connect to a .sf file
        location ~ [^/]\.sf(/|$) {
             fastcgi_split_path_info ^(.+?\.sf)(/.*)$;
             if (!-f $document_root$fastcgi_script_name) {
                 return 404;
             }

             # Conenct via IPv4
             #fastcgi_pass   127.0.0.1:8123;
             
             # Connect via unix sockets
             fastcgi_pass    unix:/var/run/saffire.socket;
             
             fastcgi_index   index.sf;
             include         fastcgi_params;
        }
     }

Make sure your fastcgi_pass points to the correct unix-socket or TCP ip:port. 

Also, add a index.sf file to your document_root (/usr/share/nginx/www/index.sf, in our case):

    import io;
    io.print("Hello world from FastCGI");

After restarting nginx, and starting the saffire fastcgi daemon you can try and connect to it through your webbrowser:

      http://127.0.0.1:80/index.sf
      
This should output our hello world file.


HTTP Libraries
--------------
While not avaialble yet, there will be some http libraries to help you with input/output through fastcgi. With these libraries you can process the users request and creates respones.