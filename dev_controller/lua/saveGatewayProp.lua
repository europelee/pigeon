--saveGatewayProp lua script
--europelee
--2016-03-23

local bExist = redis.pcall('sismember', KEYS[1], ARGV[1])

if bExist == 1 then
    local gwProp = cjson.decode(ARGV[2])
    local onLine = gwProp['online']
    local gwInstPropKey = KEYS[2] 
    local ret = redis.call('hmset', gwInstPropKey, 'online', onLine) 
    return ret 
end

return nil 
