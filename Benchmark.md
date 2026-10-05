# DreiZehn-Script Benchmark 

I used my Benchmark Script from ElfScript:

- Lua 5.5.1: 1.243u 0.002s 0:01.25 99.2%     0+0k 0+0io 0pf+0w
- Elfscript 0.7c: 1.354u 0.002s 0:01.36 99.2%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.7c): 5.393u 0.006s 0:05.42 99.4%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn 0.6b: 8.300u 0.002s 0:08.33 99.6%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn 0.6a: 11.218u 0.012s 0:11.28 99.4%    0+0k 0+0io 0pf+0w
- 🐢 DreiZehn 0.4c: 12.478u 0.002s 0:12.52 99.6%    0+0k 0+0io 0pf+0w
- python 3: 15.768u 0.005s 0:15.83 99.5%    0+0k 0+0io 0pf+0w. 
- 🐢 DreiZehn 0.4a: 21.160u 0.002s 0:21.23 99.6%    0+0k 0+0io 0pf+0w
- 🐢 DreiZehn 0.3: 23.020u 0.002s 0:23.07 99.7%    0+0k 0+0io 0pf+0w
- 🐢 DreiZehn 0.1: 26.907u 0.006s 0:27.00 99.6%    0+0k 0+0io 0pf+0w
- OGE3D (my Torque3D based on 3.10) : 33.268u 0.299s 0:33.61 99.8%  0+0k 0+24io 0pf+0w

🐢 == Noderunner (pre bytecode)- using "global/scope variables"- not bad for that ;) 

👾 == bytecode in porting mode ....

🚀 == bytecode in place 


## Script on Version 0.6:

```
# Benchmark-Test for DreiZehn Script (0.6)
JLOOPS = 25
ILOOPS = 1000000
HLOOPS = 1000000 / 2

globalX = 0

forRange j JLOOPS
    forRange i ILOOPS
        globalX++
    end
    print  "SUM (++) IS: " globalX

    forRange i ILOOPS
        globalX--
    end
    print  "SUM (--) IS: " globalX

    globalX = 66
    print  "set Sum to 66 == " (globalX * 1)

    forRange i ILOOPS
        globalX *= (i + 1)
        globalX /= (i + 1)
    end
    print  "SUM (*/ %i+1) IS: " globalX

    ran = 0.0
    forRange i HLOOPS
        ran = math::randomf * i
        globalX -= ran
        globalX += ran
    end
    print  "last ran" ran
    print  "SUM (rand +-) IS: " globalX
end

globalX = globalX - 33
print "---------------------"
print "---------------------"
print  "Final sum should be 33 == " globalX
print "---------------------"
print "---------------------"

```

## Field Benchmark
Same as before but with Vector3Object field. 

Note on 0.5b: While Variables  uses fast "++"/"--" fields only have the slower assign

Note on 0.5c: Added inline OP and OP assign :) Nearly as fast as global var

- 🐢 DreiZehn(0.7c): 7.448u 0.005s 0:07.47 99.5%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn 0.6b: 8.945u 0.003s 0:08.98 99.5%     0+0k 8+0io 0pf+0w
- 🐢 DreiZehn 0.6a: 11.743u 0.004s 0:11.78 99.6%    0+0k 0+0io 0pf+0w
- 🐢 DreiZehn 0.5c: 13.057u 0.004s 0:13.09 99.6%    0+0k 0+0io 0pf+0w
- 🐢 DreiZehn 0.5b: 19.794u 0.004s 0:19.85 99.6%    0+0k 0+0io 0pf+0w


## Counting to one Billion 

stupid iterator test ;)

- 🐢 DreiZehn(0.7c): 0.334u 0.004s 0:00.34 97.0%     0+0k 0+0io 0pf+0w
- ElfScript 0.7a: 2.688u 0.003s 0:02.69 99.6% 0+0k 0+0io 0pf+0w
- Lua (5.5.0): 3.504u 0.003s 0:03.52 99.4% 0+0k 0+0io 0pf+0w
- PHP (8.5.8): 3.644u 0.019s 0:03.66 99.7% 0+0k 0+0io 0pf+0w
- 👾 DreiZehn(0.4a): 8.512u 0.000s 0:08.54 99.6%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.3): 14.335u 0.002s 0:14.36 99.7%    0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.1): 29.409u 0.001s 0:29.48 99.7%    0+0k 0+0io 0pf+0w
- Python 3 (3.14.6): 40.648u 0.006s 0:40.71 99.8% 0+0k 0+0io 0pf+0w
- ruby 3.4.10: 55.675u 0.023s 0:55.79 99.8% 0+0k 0+0io 0pf+0w
- Duktape (2.7.0 RelWithDeb): 179.464u 0.000s 2:59.77 99.8% 0+0k 0+0io 0pf+0w
- 🌩️ ChaiScript ( v6.1.0 RelWithDeb (*5)): Segmentation fault (core dumped) after: 190.456u 0.148s 3:11.52 99.5% 0+0k 1312+0io 9pf+0w

DreiZehn beat Python 3 and Ruby here *lol*. 

```
i = 0
for i 1 1000000000
end
```



