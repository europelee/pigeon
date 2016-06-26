--saveGatewayProp lua script
--europelee
--2016-03-23

local bExist = redis.pcall('sismember', KEYS[1], ARGV[1])

if bExist == 1 then
    local gwProp = cjson.decode(ARGV[2])
    local onLine = gwProp['online']
    local gwInstPropKey = KEYS[2] 
    local ret = redis.call('hmset', gwInstPropKey, 'online', onLine) 
    --treat gwInstPropKey as a live&tmp.
    redis.call('expire', gwInstPropKey, 60)
    return ret 
end

return nil 
