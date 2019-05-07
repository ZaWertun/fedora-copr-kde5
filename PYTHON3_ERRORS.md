kf5-ki18n-devel
===============

+ Error while running with python3:

    > python3 /usr/lib64/cmake/KF5I18n/ts-pmap-compile.py                                                                                                                                                                                                      0.009s (0) 14:20:13
    Traceback (most recent call last):
      File "/usr/lib64/cmake/KF5I18n/ts-pmap-compile.py", line 297, in <module>
        main()
      File "/usr/lib64/cmake/KF5I18n/ts-pmap-compile.py", line 279, in main
        error("usage: %s INPUT_FILE OUTPUT_FILE" % cmdname)
      File "/usr/lib64/cmake/KF5I18n/ts-pmap-compile.py", line 23, in error
        sys.stderr.write(("%s: error: %s\n" % (cmdname, msg)).encode(lenc))
    TypeError: write() argument must be str, not bytes

+ ts-pmap-compile.py - no shebang
