4/12/2025
perft : 1 | 20 nodes in 3.09e-05 seconds | 0.647249m nodes/sec
perft : 2 | 400 nodes in 0.0002803 seconds | 1.42704m nodes/sec
perft : 3 | 8902 nodes in 0.0049345 seconds | 1.80403m nodes/sec
perft : 4 | 197281 nodes in 0.104091 seconds | 1.89527m nodes/sec
perft : 5 | 4865609 nodes in 2.45868 seconds | 1.97895m nodes/sec

Finalised Move generation correctness, testing on a variety of positions. Quite happy with logic, now looking to optimise speed.
Aim is 50 million nps. Currently at ~1.9m nps, likely from costly vector resizing operations as I'm still using push back. 
Will switch to fixed size array iterations now.


After using fixed sized arrays, it generates 6 million nodes per second, a noticeable improvement:
perft : 1 | 20 nodes in 2.41e-05 seconds | 0.829876m nodes/sec
perft : 2 | 400 nodes in 7.7e-05 seconds | 5.19481m nodes/sec
perft : 3 | 8902 nodes in 0.001448 seconds | 6.14779m nodes/sec
perft : 4 | 197281 nodes in 0.0349081 seconds | 5.65144m nodes/sec
perft : 5 | 4865609 nodes in 0.817728 seconds | 5.95015m nodes/sec

However still far from the target. After removing pseudolegal generation and generating only legal moves: 

: 1 | 20 nodes in 6.5e-06 seconds | 3.07692m nodes/sec
perft : 2 | 400 nodes in 0.0002122 seconds | 1.88501m nodes/sec
perft : 3 | 8902 nodes in 0.0014605 seconds | 6.09517m nodes/sec
perft : 4 | 197281 nodes in 0.0357042 seconds | 5.52543m nodes/sec
perft : 5 | 4865609 nodes in 0.849997 seconds | 5.72427m nodes/sec

Not much better. Looking into other engines to identify major bottlenecks.