<this file is UTF-8 coding>

这里边有ugly的东西, 有历史遗留问题, 本次u-boot的移植并不能解决, 只能继承下来, 某些地方为了兼容甚至做得更ugly.

这个目录保存负责parse下面几个XML的代码
        config.xml      (主要是各个驱动的配置, 内容各种各样)
        bin_cfg.xml     (也是驱动的配置, 启动阶段用的, 主要是LCD相关, 跟config.xml分离是为了parse速度, 尽快亮屏)
        pinctrl.xml     (MFP配置)
        gpiocfg.xml     (GPIO配置)
702C是使用XML保存板级的配置的, 后期的其它项目才改用device tree

xml的parse主要是依赖mxml的库, 输入是XML文本格式的字符串, parse完成后是内存里边的一张链表构成的树.
boot阶段对那些XML解析完后, 会保存4张链表树备后用.
call kernel时会将config.xml和bin_cfg.xml对应的链表通过ATAG方式传递给kernel. (ATAG_XML ...)
这里就有2个问题:
    1. ATAG机制只能传递一段紧凑的数据, 而XML parse过后是一个松散的链表树
    2. 链表是地址相关的, 传到内核后地址空间变了, 不能直接使用

对于问题1, 以前的boot是绕过去的. 老boot使用的是一个简单的线性内存分配器, parse XML时申请的一串node基本上是连续分配的, 也就是, 在内存中它们基本是紧挨着的, parse过后, 得到的链表在内存中其实就是连续的一整块, 下一个malloc所得到的地址, 可以认为是之前parse所生成的链表块的结束地址. 为了保险, 一个XML parse过后, 会再malloc一大块buffer, 这个buffer的结束地址减去XML链表的第一个node的地址, 就认为是这个XML链表(块)的size. ATAG传递时, 附带data的地址是XML链表的第一个node的地址, 而size就取上述方法得到的size.

而第2个问题, 在进入到内核, 将由ATAG拿到的链表(块)整体copy保存后, 要进行重定位才能使用. 所谓的重定位, 就是一边用算得的偏移修正node的prev/next指针, 一边继续遍历, 直到处理完整条链表.

在u-boot这边, 上面1的方法是不可行的. u-boot的内存分配器不保证连续分配的buffers有线性连续的地址. 事实上依赖于这样的假定本来就很危险, 也不具备移植性. 曾经考虑过2个解法:
        1. 不传递链表, 改为传递地址无关的XML源字节串, kernel内重新parse一遍.
                这个做法最clean, 但是1会增加启动时间, 2要改kernel要人力/review/评审/@#*%$^*...
        2. 维持原来的做法, 想办法使得mxml调的malloc是简单线性分配.
折中后结果是2. 改mxml库, 原malloc调用改为mxml_malloc, 一个简单线性分配器. 在开始parse XML前, 先从u-boot malloc一块大的buffer, 作为那个简单线性分配器的内存池.

这样改之后, mxml库就是这个板型专用的东西了, 只能放到板型目录里边了.


mxml库改动点:
1. 实现mxml_heap, 使用最简单的顺序分配. 实现 mxml_malloc() mxml_calloc() mxml_realloc() mxml_mfree()
2. 将mxml库内所有malloc calloc realloc free 改为调用mxml_heap里边的.
3. mxml不使用标准的strdup, 因为会间接调用系统的malloc. 改用mxml自己实现的_mxml_strdup即可(不定义HAVE_STRDUP)
4. 不定义HAVE_PTHREAD_H, 免得节外生枝.
5. 调用 mxmlSaveAllocString() 之类的返回内部allocated buffer的函数后, free的时候要调用mxml_mfree(), 而不是系统的free()
