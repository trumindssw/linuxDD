Sometimes, drivers run in a loop or under heavy load, generating too many printk messages. This floods the logs and slows the system. To avoid this, the kernel provides rate-limited variants of printk().

printk_ratelimit works by tracking how many messages are sent to the console.
When the level of output exceeds a threshold, printk_ratelimit starts returning 0 and causing messages to be dropped.

Use printk_ratelimited() to avoid log flooding.

Very useful in fast loops, high-frequency paths.

The behavior of printk_ratelimit can be customized by modifying /proc/sys/kernel/
printk_ratelimit (the number of seconds to wait before re-enabling messages) and are
/proc/sys/kernel/printk_ratelimit_burst (the number of messages accepted before rate-
limiting).