# DreiZehn-Script Benchmark 

I used my Benchmark Script from ElfScript:

- Lua 5.5.1: 1.243u 0.002s 0:01.25 99.2%     0+0k 0+0io 0pf+0w
- Elfscript 0.7c: 1.354u 0.002s 0:01.36 99.2%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn 0.4c: 12.478u 0.002s 0:12.52 99.6%    0+0k 0+0io 0pf+0w
- python 3: 15.768u 0.005s 0:15.83 99.5%    0+0k 0+0io 0pf+0w. 
- 🐢 DreiZehn 0.4a: 21.160u 0.002s 0:21.23 99.6%    0+0k 0+0io 0pf+0w
- 🐢 DreiZehn 0.3: 23.020u 0.002s 0:23.07 99.7%    0+0k 0+0io 0pf+0w
- 🐢 DreiZehn 0.1: 26.907u 0.006s 0:27.00 99.6%    0+0k 0+0io 0pf+0w
- OGE3D (my Torque3D based on 3.10) : 33.268u 0.299s 0:33.61 99.8%  0+0k 0+24io 0pf+0w

🐢 == Noderunner (pre bytecode)- not bad for that ;) 

👾 == bytecode in porting mode ....

🚀 == bytecode in place 


## Script on Version 0.1:

```
# Benchmark-Test for DreiZehn Engine
JLOOPS = 25
ILOOPS = 1000000

localX = 0

for j 0 (JLOOPS - 1)
    for i 0 (ILOOPS - 1)
        localX = localX + 1
    end
    print (concat "SUM (++) IS: " localX)

    for i 0 (ILOOPS - 1)
        localX = localX - 1
    end
    print (concat "SUM (--) IS: " localX)

    localX = 66
    print (concat "set Sum to 66 == " (localX * 1))

    for i 0 (ILOOPS - 1)
        localX = localX * (i + 1)
        localX = localX / (i + 1)
    end
    print (concat "SUM (*/ %i+1) IS: " localX)

    # TEST comment
    limit = (ILOOPS / 2) - 1
    ran = 0.0
    for i 0 limit
        ran = random * i
        localX = localX - ran
        localX = localX + ran
    end
    print (concat "last ran" ran)
    print (concat "SUM (rand +-) IS: " localX)
end

localX = localX - 33
print "---------------------"
print "---------------------"
print (concat "Final sum should be 33 == " localX)
print "---------------------"
print "---------------------"
```

## Field Benchmark
Same as before but with Vector3Object field. 

Note on 0.5b: While Variables  uses fast "++"/"--" fields only have the slower assign

- 🐢 DreiZehn 0.5b: 19.794u 0.004s 0:19.85 99.6%    0+0k 0+0io 0pf+0w




## Counting to one Billion 

stupid iterator test ;)

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



