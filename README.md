# idebug
PHP Internal Debug Tool. Helps you discovery PHP's data structures and algorithms.

### 简介
idebug 是一个帮助 PHP 内核或扩展开发人员的调试工具，可以通过该工具查看 PHP 内部存储，深入了解 PHP 的工作原理。在使用这个工具之前，希望你先对 PHP 内核结构及源码有所了解，方能更好地使用本工具。

### 初衷
开发本工具的目的，是因为在开发 PHP 扩展中经常会遇到段错误，使用 GDB 调试太繁琐，而另一个成熟的内核调试工具 xdebug 没有我需要的功能，自己编写一个调试工具不仅满足了自身需要，还可以更加了解 PHP 内核数据结构与算法。
