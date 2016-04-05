--cacheUPRepSub lua script
--europelee
--2016-04-05

local bRet = redis.pcall('srem', KEYS[1], ARGV[1])

return bRet
