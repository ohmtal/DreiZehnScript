# DreiZehn-Script Benchmark 

I used my Benchmark Script from ElfScript:

- 👾 DreiZehn(0.9e): 1.187u 0.003s 0:01.19 99.1%     0+0k 0+0io 0pf+0w
- Lua 5.5.1: 1.243u 0.002s 0:01.25 99.2%     0+0k 0+0io 0pf+0w
- 👾 DreiZehn(0.9f): 1.265u 0.004s 0:01.27 99.2%     0+0k 0+0io 0pf+0w
- 👾 DreiZehn(0.9d): 1.252u 0.006s 0:01.26 99.2%     0+0k 0+0io 0pf+0w
- Elfscript 0.7c: 1.354u 0.002s 0:01.36 99.2%     0+0k 0+0io 0pf+0w
- 👾 DreiZehn(0.9a): 1.410u 0.005s 0:01.42 99.2%     0+0k 0+0io 0pf+0w 
- 👾 DreiZehn(0.9b ?!): 1.873u 0.004s 0:01.88 99.4%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.9e): 2.594u 0.006s 0:02.61 99.2%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.9d): 3.463u 0.004s 0:03.48 99.4%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.8b): 3.306u 0.006s 0:03.32 99.3%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.8e): 3.417u 0.001s 0:03.42 99.7%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.8a): 3.835u 0.005s 0:03.86 99.2%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.7c): 5.393u 0.006s 0:05.42 99.4%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.7f): 5.995u 0.001s 0:06.02 99.5%     0+0k 0+0io 0pf+0w
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

Note 0.8b is 0.8a but using -02 optimized instead of -03.

## StarField II 4200 Stars (rectangle mode):

i need to write a better Benchmark here but for now its only a info for me.

0.9e added a 3000 loop bench. 

it can happen, that when i restart the prog that it's only peek 310 not 340
- 👾 DreiZehn(0.9f): 4.938u 0.208s 0:05.37 95.5% (peek: 680FPS, low: 610FPS)
- 👾 DreiZehn(0.9e): 5.391u 0.200s 0:05.82 96.0% (peek: 600FPS, low: 500FPS)
- 🐢 DreiZehn(0.9f): 7.477u 0.230s 0:07.91 97.3% (peek: 440FPS, low: 425FPS)
- 🐢 DreiZehn(0.9e): 8.672u 0.218s 0:09.16 96.9%(peek: 350 FPS, low: 325 FPS)
- 👾 DreiZehn(0.9d): peek: 500 FPS, low: 400 FPS
- 🐢 DreiZehn(0.9c): peek: 315 FPS, low: 280 FPS - bummer
- 🐢 DreiZehn(0.8e): peek: 340 FPS, low: 325 FPS


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

- 👾 DreiZehn(0.9f): 1.780u 0.007s 0:01.79 99.4%     0+0k 0+0io 0pf+0w
- 👾 DreiZehn(0.9d): 2.709u 0.001s 0:02.72 99.2%     0+0k 0+0io 0pf+0w
- 👾 DreiZehn(0.9e): 2.774u 0.004s 0:02.78 99.6%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.9e): 3.234u 0.003s 0:03.24 99.6%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.9c): 3.709u 0.004s 0:03.72 99.4%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.8e): 3.969u 0.005s 0:03.98 99.4%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.8b): 5.150u 0.003s 0:05.16 99.8%     0+0k 0+0io 0pf+0w 
- 🐢 DreiZehn(0.8a): 5.529u 0.003s 0:05.55 99.4%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.7c): 7.448u 0.005s 0:07.47 99.5%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.7f): 7.817u 0.003s 0:07.85 99.4%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn 0.6b: 8.945u 0.003s 0:08.98 99.5%     0+0k 8+0io 0pf+0w
- 🐢 DreiZehn 0.6a: 11.743u 0.004s 0:11.78 99.6%    0+0k 0+0io 0pf+0w
- 🐢 DreiZehn 0.5c: 13.057u 0.004s 0:13.09 99.6%    0+0k 0+0io 0pf+0w
- 🐢 DreiZehn 0.5b: 19.794u 0.004s 0:19.85 99.6%    0+0k 0+0io 0pf+0w


## Recursive function call scripts/test_fibonacci.13 (33)
```
fn fib n
    if n <= 1; return n;;
    return (fib (n - 1)) + (fib (n - 2))
end

num = 33
print "running fib with number:" num
print "Fib recursiv" num "=" (fib num)
```
- php (0.5.10): 0.252u 0.008s 0:00.26 96.1%     0+0k 0+0io 0pf+0w
- lua (Lua 5.5.1): 0.325u 0.002s 0:00.33 96.9%     0+0k 0+0io 0pf+0w
- 👾 DreiZehn(0.9e): 0.337u 0.004s 0:00.34 97.0%     0+0k 0+0io 0pf+0w
- 👾 DreiZehn(0.9f):  0.346u 0.001s 0:00.34 100.0%    0+0k 0+0io 0pf+0w
- ruby (3.4.10): 0.447u 0.013s 0:00.48 93.7%     0+0k 3392+0io 67pf+0w
- 👾 DreiZehn(0.9d): 0.646u 0.004s 0:00.65 98.4%     0+0k 0+0io 0pf+0w
- Python (3.14.7): 0.649u 0.006s 0:00.65 98.4%     0+0k 0+0io 0pf+0w
- ElfScript 0.8a: 1.199u 0.001s 0:01.20 99.1%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.8e): 2.270u 0.002s 0:02.27 100.0%    0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.9c): 2.296u 0.003s 0:02.30 99.5%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.8a): 2.396u 0.001s 0:02.40 99.5%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.8b): 2.647u 0.004s 0:02.66 99.2%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.7f): 2.339u 0.001s 0:02.34 99.5%     0+0k 0+0io 0pf+0w
- 💩 ElfScript 0.8: 4.446u 0.003s 0:04.46 99.5%     0+0k 0+0io 0pf+0w

DreiZehn 0.7f very cool for a AST-Interpreter :D 

ElfScript 0.8 sucks here must be optimized for script function calls! 

ElfScript 0.8a all night long hackfest for inline function call, damn i hoped i get
under 0.5 sec ;)


## Counting to one Billion 

stupid iterator test ;)
- 🐢 DreiZehn(0.7f): 0.333u 0.002s 0:00.33 100.0%    0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.7c): 0.334u 0.004s 0:00.34 97.0%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.8e): 0.814u 0.004s 0:00.82 98.7%     0+0k 0+0io 0pf+0w 
- 🐢 DreiZehn(0.8b): 0.982u 0.004s 0:00.98 100.0%    0+0k 0+0io 0pf+0w
- ElfScript 0.7a: 2.688u 0.003s 0:02.69 99.6% 0+0k 0+0io 0pf+0w
- 👾 DreiZehn(0.9a): 2.740u 0.005s 0:02.75 99.6%     0+0k 0+0io 0pf+0w
- 👾 DreiZehn(0.9e): 3.252u 0.004s 0:03.26 99.6%     0+0k 0+0io 0pf+0w
- 👾 DreiZehn(0.9d): 3.258u 0.007s 0:03.27 99.3%     0+0k 0+0io 0pf+0w
- Lua (5.5.0): 3.504u 0.003s 0:03.52 99.4% 0+0k 0+0io 0pf+0w
- PHP (8.5.8): 3.644u 0.019s 0:03.66 99.7% 0+0k 0+0io 0pf+0w
- 👾 DreiZehn(0.4a): 8.512u 0.000s 0:08.54 99.6%     0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.3): 14.335u 0.002s 0:14.36 99.7%    0+0k 0+0io 0pf+0w
- 🐢 DreiZehn(0.1): 29.409u 0.001s 0:29.48 99.7%    0+0k 0+0io 0pf+0w
- Python 3 (3.14.6): 40.648u 0.006s 0:40.71 99.8% 0+0k 0+0io 0pf+0w
- ruby 3.4.10: 55.675u 0.023s 0:55.79 99.8% 0+0k 0+0io 0pf+0w
- Duktape (2.7.0 RelWithDeb): 179.464u 0.000s 2:59.77 99.8% 0+0k 0+0io 0pf+0w
- 🌩️ ChaiScript ( v6.1.0 RelWithDeb (*5)): Segmentation fault (core dumped) after: 190.456u 0.148s 3:11.52 99.5% 0+0k 1312+0io 9pf+0w


```
i = 0
for i 1 1000000000
end
print "I=" i
```



