Overview

developer portal: an  IOT cloud service for developers, and it provides REST APIs for creating and managing device type.

Setup

1.dependencies:

redis 3.0 (mine configure file redis_6397.conf like below)
    
    bind 127.0.0.1
    port 6379
    loglevel debug
    logfile iot-redis.log
    
    appendonly yes
    appendfilename appendonly.aof
    # appendfsync always
    appendfsync everysec
    # appendfsync no
    no-appendfsync-on-rewrite yes
    auto-aof-rewrite-percentage 100
    auto-aof-rewrite-min-size 64mb

    
and need install express4.13.3, node_redis, async, express-session, jquery.

2.cd developer_portal directory, then install its node_modules with 'npm install'.

3.you can run with 'DEBUG=developer_portal npm start'(also start redis-server like './redis-server ./redis_6397.conf &')



-----------------------------------------------------------------------------------------------------------------
    its REST APIs(you can send request with them and query results on developer portal settings):

-----------------------------------------------------------------------------------------------------------------
    1. create device type
        post  /v0.1/iot/devices

        request body format: json
        eg:{"devtype":"q46", "manufacturer":"euputddfds", "version":"0.0.1", "description":"hi, eu"}
        response: if succ, return http status code 201
-----------------------------------------------------------------------------------------------------------------
    2. query device type info
        get /v0.1/iot/devices/<devtype>

        response body format: json
        eg:{"manufacturer": null,  "version": "0.0.1",  "description": "hi, eu,new version come on now!\n"}
-----------------------------------------------------------------------------------------------------------------
    3. get device type list
        get  /v0.1/iot/devices

        response body format: json
        eg: [ "q56",  "q66",  "q46",  "q36",  "q6", devTypeName]
-----------------------------------------------------------------------------------------------------------------
    4. update device type info
        put  /v0.1/iot/devices/<devtype>

        request body format: json
        eg:{"manufacturer":"euputddfds", "version":"0.0.1", "description":"hi, eu"}
        response: if succ, return http status code 200
-----------------------------------------------------------------------------------------------------------------
    5. create device meta
        post /v0.1/iot/devices/<devtype>/meta

        request body format: json
        eg:{"id":"string", "pos":"float"} //it means <devtype> has two properties: field 'id',type:string, and field 'pos', type float
        response: if succ, return http status code 201
-----------------------------------------------------------------------------------------------------------------
    6. query device meta
        get /v0.1/iot/devices/<devtype>/meta

        response body format: json
        eg:{"id":"string", "pos":"float"}
-----------------------------------------------------------------------------------------------------------------
    7. update device meta
        put /v0.1/iot/devices/<devtype>/meta

        request body format: json
        eg:{"pos":"int"}
    response: if succ, return http status code 200
-----------------------------------------------------------------------------------------------------------------
    8. delete device meta
        delete /v0.1/iot/devices/<devtype>/meta

        response: if succ, return http status code 200
-----------------------------------------------------------------------------------------------------------------
    9. delete device type(it will delete devtype info (also include its meta))
        delete /v0.1/iot/devices/<devtype>

        response: if succ, return http status code 200
-----------------------------------------------------------------------------------------------------------------

you can test the above APIs with postman, but first get apikey as authorization by the below steps:

1. you must sign up on developer portal
2. then go to personal profile, copy App key as value of your REST APIs http request header 'apikey'.

