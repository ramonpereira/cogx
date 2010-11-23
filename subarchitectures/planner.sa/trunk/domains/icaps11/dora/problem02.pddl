(define (scenario dora-test-world)

  (:domain dora-test-100)

  (:common
   (:objects  dora - robot
              human - human
              p1a p2a p3a p4a p5a p6a
              p1b p2b p3b p4b
              p1c p2c p3c - place
              ra rb rc - room
              kitchen office living_room - category
              kitchen_pantry dining_room bathroom - category
              cornflakes table mug oven fridge book board-game - label
              obj_cornflakes obj_table_a obj_table_b obj_table_c obj_mug_a obj_mug_b obj_mug_c obj4 obj5 obj6 obj7 - visualobject
              )

   (:init  (= (is-in dora)  p1a)

           (= (in-room p1a) ra)
           (= (in-room p2a) ra)
           (= (in-room p3a) ra)
           (= (in-room p4a) ra)
           (= (in-room p5a) ra)
           (= (in-room p6a) ra)
           (= (in-room p1b) rb)
           (= (in-room p2b) rb)
           (= (in-room p3b) rb)
           (= (in-room p4b) rb)
           (= (in-room p1c) rc)
           (= (in-room p2c) rc)
           (= (in-room p3c) rc)
           (connected p1a p2a)
           (connected p1a p3a)
           (connected p3a p4a)
           (connected p4a p5a)
           (connected p5a p1a)
           (connected p2a p6a)
           (connected p6a p1a)

           (connected p2a p1b)
           (connected p1b p2b)
           (connected p2b p3b)
           (connected p3b p4b)
           (connected p4b p1b)

           (connected p3a p1c)
           (connected p1c p2c)
           (connected p2c p3c)
           (connected p3c p1c)

           (= (difficulty cornflakes) medium)
           (= (difficulty mug) hard)
           (= (difficulty table) easy)
           (= (difficulty oven) easy)
           (= (difficulty fridge) easy)
           (= (difficulty book) medium)
           (= (difficulty board-game) medium)

           (= (label obj_cornflakes) cornflakes)
           (= (label obj_table_a) table)
           (= (label obj_table_b) table)
           (= (label obj_table_c) table)
           (= (label obj_mug_a) mug)
           (= (label obj_mug_b) mug)
           (= (label obj_mug_c) mug)
           (= (label obj4) oven)
           (= (label obj5) fridge)
           (= (label obj6) book)
           (= (label obj7) board-game)

           (probabilistic  0.1000  (assign (category rb) bathroom)
                           0.1000  (and  (assign (category rb) living_room)
                                         (probabilistic  0.7000  (and  (assign (ex-in-room table rb) true)
                                                                       (probabilistic  0.5000  (assign (is-in obj_table_b) p2b)
                                                                                       0.4000  (assign (is-in obj_table_b) p3b)
                                                                                       0.1000  (assign (is-in obj_table_b) p4b)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.3000  (assign (ex-in-room book rb) true))
                                         (probabilistic  0.6000  (and  (assign (ex-in-room mug rb) true)
                                                                       (probabilistic  0.3000  (assign (is-in obj_mug_b) p2b)
                                                                                       0.4000  (assign (is-in obj_mug_b) p3b)
                                                                                       0.1000  (assign (is-in obj_mug_b) p1b)
                                                                                       0.2000  (assign (is-in obj_mug_b) p4b)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.7000  (assign (ex-in-room board-game rb) true))
                                         (probabilistic  0.1000  (and  (assign (ex-in-room cornflakes rb) true)
                                                                       (probabilistic  0.3000  (assign (is-in obj_cornflakes) p2b)
                                                                                       0.4000  (assign (is-in obj_cornflakes) p3b)
                                                                                       0.1000  (assign (is-in obj_cornflakes) p1b)
                                                                                       0.2000  (assign (is-in obj_cornflakes) p4b)
                                                                                       )
                                                                       )
                                                         )
                                         )
                           0.2000  (and  (assign (category rb) office)
                                         (probabilistic  0.9000  (and  (assign (ex-in-room table rb) true)
                                                                       (probabilistic  0.5000  (assign (is-in obj_table_b) p2b)
                                                                                       0.4000  (assign (is-in obj_table_b) p3b)
                                                                                       0.1000  (assign (is-in obj_table_b) p4b)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.8000  (assign (ex-in-room book rb) true))
                                         (probabilistic  0.9000  (and  (assign (ex-in-room mug rb) true)
                                                                       (probabilistic  0.3000  (assign (is-in obj_mug_b) p2b)
                                                                                       0.4000  (assign (is-in obj_mug_b) p3b)
                                                                                       0.1000  (assign (is-in obj_mug_b) p1b)
                                                                                       0.2000  (assign (is-in obj_mug_b) p4b)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.1000  (assign (ex-in-room fridge rb) true))
                                         (probabilistic  0.1000  (and  (assign (ex-in-room cornflakes rb) true)
                                                                       (probabilistic  0.3000  (assign (is-in obj_cornflakes) p2b)
                                                                                       0.4000  (assign (is-in obj_cornflakes) p3b)
                                                                                       0.1000  (assign (is-in obj_cornflakes) p1b)
                                                                                       0.2000  (assign (is-in obj_cornflakes) p4b)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.2000  (assign (ex-in-room board-game rb) true))
                                         )
                           0.6000  (and  (assign (category rb) kitchen)
                                         (probabilistic  0.9000  (and  (assign (ex-in-room table rb) true)
                                                                       (probabilistic  0.5000  (assign (is-in obj_table_b) p2b)
                                                                                       0.4000  (assign (is-in obj_table_b) p3b)
                                                                                       0.1000  (assign (is-in obj_table_b) p4b)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.2000  (assign (ex-in-room book rb) true))
                                         (probabilistic  0.8000  (and  (assign (ex-in-room mug rb) true)
                                                                       (probabilistic  0.3000  (assign (is-in obj_mug_b) p2b)
                                                                                       0.4000  (assign (is-in obj_mug_b) p3b)
                                                                                       0.1000  (assign (is-in obj_mug_b) p1b)
                                                                                       0.2000  (assign (is-in obj_mug_b) p4b)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.7000  (assign (ex-in-room fridge rb) true))
                                         (probabilistic  0.8000  (and  (assign (ex-in-room cornflakes rb) true)
                                                                       (probabilistic  0.3000  (assign (is-in obj_cornflakes) p2b)
                                                                                       0.4000  (assign (is-in obj_cornflakes) p3b)
                                                                                       0.1000  (assign (is-in obj_cornflakes) p1b)
                                                                                       0.2000  (assign (is-in obj_cornflakes) p4b)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.8000  (and  (assign (ex-in-room oven rb) true)
                                                                       (probabilistic  0.1000  (assign (is-in obj4) p2b)
                                                                                       0.2000  (assign (is-in obj4) p3b)
                                                                                       0.7000  (assign (is-in obj4) p4b)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.1000  (assign (ex-in-room board-game rb) true))
                                         )
                           )
           (probabilistic  0.7000  (and  (assign (category ra) living_room)
                                         (probabilistic  0.6000  (and  (assign (ex-in-room mug ra) true)
                                                                       (probabilistic  0.0500  (assign (is-in obj_mug_a) p1a)
                                                                                       0.2500  (assign (is-in obj_mug_a) p5a)
                                                                                       0.2500  (assign (is-in obj_mug_a) p6a)
                                                                                       0.1500  (assign (is-in obj_mug_a) p3a)
                                                                                       0.0500  (assign (is-in obj_mug_a) p2a)
                                                                                       0.2500  (assign (is-in obj_mug_a) p4a)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.3000  (assign (ex-in-room book ra) true))
                                         (probabilistic  0.7000  (and  (assign (ex-in-room table ra) true)
                                                                       (probabilistic  0.3000  (assign (is-in obj_table_a) p4a)
                                                                                       0.3000  (assign (is-in obj_table_a) p6a)
                                                                                       0.2000  (assign (is-in obj_table_a) p1a)
                                                                                       0.2000  (assign (is-in obj_table_a) p5a)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.1000  (and  (assign (ex-in-room cornflakes ra) true)
                                                                       (probabilistic  0.0500  (assign (is-in obj_cornflakes) p1a)
                                                                                       0.2500  (assign (is-in obj_cornflakes) p5a)
                                                                                       0.2500  (assign (is-in obj_cornflakes) p6a)
                                                                                       0.1500  (assign (is-in obj_cornflakes) p3a)
                                                                                       0.0500  (assign (is-in obj_cornflakes) p2a)
                                                                                       0.2500  (assign (is-in obj_cornflakes) p4a)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.7000  (assign (ex-in-room board-game ra) true))
                                         )
                           0.2000  (and  (assign (category ra) office)
                                         (probabilistic  0.9000  (and  (assign (ex-in-room mug ra) true)
                                                                       (probabilistic  0.0500  (assign (is-in obj_mug_a) p1a)
                                                                                       0.2500  (assign (is-in obj_mug_a) p5a)
                                                                                       0.2500  (assign (is-in obj_mug_a) p6a)
                                                                                       0.1500  (assign (is-in obj_mug_a) p3a)
                                                                                       0.0500  (assign (is-in obj_mug_a) p2a)
                                                                                       0.2500  (assign (is-in obj_mug_a) p4a)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.8000  (assign (ex-in-room book ra) true))
                                         (probabilistic  0.9000  (and  (assign (ex-in-room table ra) true)
                                                                       (probabilistic  0.3000  (assign (is-in obj_table_a) p4a)
                                                                                       0.3000  (assign (is-in obj_table_a) p6a)
                                                                                       0.2000  (assign (is-in obj_table_a) p1a)
                                                                                       0.2000  (assign (is-in obj_table_a) p5a)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.1000  (and  (assign (ex-in-room cornflakes ra) true)
                                                                       (probabilistic  0.0500  (assign (is-in obj_cornflakes) p1a)
                                                                                       0.2500  (assign (is-in obj_cornflakes) p5a)
                                                                                       0.2500  (assign (is-in obj_cornflakes) p6a)
                                                                                       0.1500  (assign (is-in obj_cornflakes) p3a)
                                                                                       0.0500  (assign (is-in obj_cornflakes) p2a)
                                                                                       0.2500  (assign (is-in obj_cornflakes) p4a)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.1000  (assign (ex-in-room fridge ra) true))
                                         (probabilistic  0.2000  (assign (ex-in-room board-game ra) true))
                                         )
                           0.1000  (and  (assign (category ra) kitchen)
                                         (probabilistic  0.8000  (and  (assign (ex-in-room mug ra) true)
                                                                       (probabilistic  0.0500  (assign (is-in obj_mug_a) p1a)
                                                                                       0.2500  (assign (is-in obj_mug_a) p5a)
                                                                                       0.2500  (assign (is-in obj_mug_a) p6a)
                                                                                       0.1500  (assign (is-in obj_mug_a) p3a)
                                                                                       0.0500  (assign (is-in obj_mug_a) p2a)
                                                                                       0.2500  (assign (is-in obj_mug_a) p4a)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.2000  (assign (ex-in-room book ra) true))
                                         (probabilistic  0.9000  (and  (assign (ex-in-room table ra) true)
                                                                       (probabilistic  0.3000  (assign (is-in obj_table_a) p4a)
                                                                                       0.3000  (assign (is-in obj_table_a) p6a)
                                                                                       0.2000  (assign (is-in obj_table_a) p1a)
                                                                                       0.2000  (assign (is-in obj_table_a) p5a)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.8000  (and  (assign (ex-in-room cornflakes ra) true)
                                                                       (probabilistic  0.0500  (assign (is-in obj_cornflakes) p1a)
                                                                                       0.2500  (assign (is-in obj_cornflakes) p5a)
                                                                                       0.2500  (assign (is-in obj_cornflakes) p6a)
                                                                                       0.1500  (assign (is-in obj_cornflakes) p3a)
                                                                                       0.0500  (assign (is-in obj_cornflakes) p2a)
                                                                                       0.2500  (assign (is-in obj_cornflakes) p4a)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.7000  (assign (ex-in-room fridge ra) true))
                                         (probabilistic  0.8000  (and  (assign (ex-in-room oven ra) true)
                                                                       (probabilistic  0.4000  (assign (is-in obj4) p4a)
                                                                                       0.4000  (assign (is-in obj4) p6a)
                                                                                       0.2000  (assign (is-in obj4) p5a)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.1000  (assign (ex-in-room board-game ra) true))
                                         )
                           )

           (probabilistic  0.3000  (assign (category rc) bathroom)
                           0.1000  (and  )
                           0.3000  (and  (assign (category rc) office)
                                         (probabilistic  0.9000  (and  (assign (ex-in-room table rc) true)
                                                                       (probabilistic  0.6000  (assign (is-in obj_table_c) p2c)
                                                                                       0.4000  (assign (is-in obj_table_c) p3c)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.9000  (and  (assign (ex-in-room mug rc) true)
                                                                       (probabilistic  0.4000  (assign (is-in obj_mug_c) p2c)
                                                                                       0.4000  (assign (is-in obj_mug_c) p3c)
                                                                                       0.2000  (assign (is-in obj_mug_c) p1c)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.8000  (assign (ex-in-room book rc) true))
                                         (probabilistic  0.1000  (and  (assign (ex-in-room cornflakes rc) true)
                                                                       (probabilistic  0.4000  (assign (is-in obj_cornflakes) p2c)
                                                                                       0.4000  (assign (is-in obj_cornflakes) p3c)
                                                                                       0.2000  (assign (is-in obj_cornflakes) p1c)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.1000  (assign (ex-in-room fridge rc) true))
                                         (probabilistic  0.2000  (assign (ex-in-room board-game rc) true))
                                         )
                           0.1000  (and  (assign (category rc) living_room)
                                         (probabilistic  0.7000  (and  (assign (ex-in-room table rc) true)
                                                                       (probabilistic  0.6000  (assign (is-in obj_table_c) p2c)
                                                                                       0.4000  (assign (is-in obj_table_c) p3c)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.6000  (and  (assign (ex-in-room mug rc) true)
                                                                       (probabilistic  0.4000  (assign (is-in obj_mug_c) p2c)
                                                                                       0.4000  (assign (is-in obj_mug_c) p3c)
                                                                                       0.2000  (assign (is-in obj_mug_c) p1c)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.3000  (assign (ex-in-room book rc) true))
                                         (probabilistic  0.7000  (assign (ex-in-room board-game rc) true))
                                         (probabilistic  0.1000  (and  (assign (ex-in-room cornflakes rc) true)
                                                                       (probabilistic  0.4000  (assign (is-in obj_cornflakes) p2c)
                                                                                       0.4000  (assign (is-in obj_cornflakes) p3c)
                                                                                       0.2000  (assign (is-in obj_cornflakes) p1c)
                                                                                       )
                                                                       )
                                                         )
                                         )
                           0.2000  (and  (assign (category rc) kitchen)
                                         (probabilistic  0.9000  (and  (assign (ex-in-room table rc) true)
                                                                       (probabilistic  0.6000  (assign (is-in obj_table_c) p2c)
                                                                                       0.4000  (assign (is-in obj_table_c) p3c)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.8000  (and  (assign (ex-in-room mug rc) true)
                                                                       (probabilistic  0.4000  (assign (is-in obj_mug_c) p2c)
                                                                                       0.4000  (assign (is-in obj_mug_c) p3c)
                                                                                       0.2000  (assign (is-in obj_mug_c) p1c)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.2000  (assign (ex-in-room book rc) true))
                                         (probabilistic  0.8000  (and  (assign (ex-in-room cornflakes rc) true)
                                                                       (probabilistic  0.4000  (assign (is-in obj_cornflakes) p2c)
                                                                                       0.4000  (assign (is-in obj_cornflakes) p3c)
                                                                                       0.2000  (assign (is-in obj_cornflakes) p1c)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.8000  (and  (assign (ex-in-room oven rc) true)
                                                                       (probabilistic  0.4000  (assign (is-in obj4) p2c)
                                                                                       0.6000  (assign (is-in obj4) p3c)
                                                                                       )
                                                                       )
                                                         )
                                         (probabilistic  0.7000  (assign (ex-in-room fridge rc) true))
                                         (probabilistic  0.1000  (assign (ex-in-room board-game rc) true))
                                         )
                           )
           )
   )

  (:agent dora
          (:goal  (and (kval dora (is-in obj_cornflakes))
                       )
                  )

          )
  )
