(define (problem ytilanoitisoporp-1231945673 )
(:domain ytilanoitisoporp)
(:objects 
o1 - boolean
o2 - boolean
o3 - boolean
o4 - boolean
o5 - boolean
o6 - boolean
o7 - boolean
o8 - boolean
o9 - boolean
o10 - boolean
o11 - boolean
o12 - boolean
o13 - boolean
)
(:init 
(off o1 )
(= (switch-cost o1 ) 5 )
(related o1 o2 )
(off o2 )
(= (switch-cost o2 ) 6 )
(related o2 o3 )
(off o3 )
(= (switch-cost o3 ) 1 )
(related o3 o4 )
(off o4 )
(= (switch-cost o4 ) 1 )
(related o4 o5 )
(off o5 )
(= (switch-cost o5 ) 5 )
(related o5 o6 )
(off o6 )
(= (switch-cost o6 ) 0 )
(related o6 o7 )
(off o7 )
(= (switch-cost o7 ) 2 )
(related o7 o8 )
(off o8 )
(= (switch-cost o8 ) 1 )
(related o8 o9 )
(off o9 )
(= (switch-cost o9 ) 0 )
(related o9 o10 )
(off o10 )
(= (switch-cost o10 ) 4 )
(related o10 o11 )
(off o11 )
(= (switch-cost o11 ) 5 )
(related o11 o12 )
(off o12 )
(= (switch-cost o12 ) 4 )
(related o12 o13 )
(off o13 )
(= (switch-cost o13 ) 1 )
(unrelated o13 )
(= (total-cost) 0)
)
(:goal 
(and 
(on o1 )
(on o2 )
(off o3 )
(on o4 )
(on o5 )
(on o6 )
(off o7 )
(off o8 )
(off o9 )
(on o10 )
(on o11 )
(on o12 )
(off o13 )
)
)
 (:metric minimize (total-cost))
)
