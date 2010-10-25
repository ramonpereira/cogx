(define (problem cogxtask)

(:domain dt-dora-test-100)

(:objects  visualobject1 visualobject0 - visualobject
           robot_8_a - robot
           place_3_a place_0_a place_6_a place_4_a place_5_a place_2_a - place
)

(:init  (connected place_6_a place_0_a)
        (in-room place_2_a room__k_a)
        (in-room place_0_a room__k_a)
        (in-room place_6_a room__k_a)
        (in-room place_5_a room__k_a)
        (connected place_6_a place_5_a)
        (connected place_0_a place_6_a)
        (label visualobject1 table)
        (connected place_2_a place_3_a)
        (connected place_0_a place_2_a)
        (connected place_5_a place_4_a)
        (connected place_4_a place_3_a)
        (label visualobject2 cornflakes)
        (connected place_2_a place_0_a)
        (is-in robot_8_a place_0_a)
        (connected place_4_a place_5_a)
        (in-room place_4_a room__k_a)
        (connected place_3_a place_2_a)
        (connected place_5_a place_6_a)
        (in-room place_3_a room__k_a)
        (connected place_3_a place_4_a)
        (probabilistic  0.3000  (and  (probabilistic  0.1000  (and  (ex-in-room cornflakes room__k_a true)
                                                                    (probabilistic  
                                                                                    0.25  (is-in visualobject2 place_4_a)
                                                                                    0.5  (is-in visualobject2 place_5_a)
                                                                                    0.25  (is-in visualobject2 place_2_a)
                                                                    )
                                                              )
                                                      0.9000  (ex-in-room cornflakes room__k_a false)
                                      )
                                      (probabilistic  0.7000  (and  (ex-in-room table room__k_a true)
                                                                    (probabilistic  
                                                                                    0.25  (is-in visualobject1 place_1_a)
                                                                                    0.5  (is-in visualobject1 place_2_a)
                                                                                    0.25  (is-in visualobject1 place_3_a)
                                                                    )
                                                              )
                                                      0.3000  (ex-in-room table room__k_a false)
                                      )
                                )
                        0.3000  (and  (probabilistic  0.1000  (and  (ex-in-room cornflakes room__k_a true)
                                                                    (probabilistic  
                                                                                    0.25  (is-in visualobject2 place_4_a)
                                                                                    0.5  (is-in visualobject2 place_5_a)
                                                                                    0.25  (is-in visualobject2 place_2_a)
                                                                    )
                                                              )
                                                      0.9000  (ex-in-room cornflakes room__k_a false)
                                      )
                                      (probabilistic  0.9000  (and  (ex-in-room table room__k_a true)
                                                                    (probabilistic  
                                                                                    0.25  (is-in visualobject1 place_1_a)
                                                                                    0.5  (is-in visualobject1 place_2_a)
                                                                                    0.25  (is-in visualobject1 place_3_a)
                                                                    )
                                                              )
                                                      0.1000  (ex-in-room table room__k_a false)
                                      )
                                )
                        0.3000  (and  (category room__k_a kitchen)
                                      (probabilistic  0.8000  (and  (ex-in-room cornflakes room__k_a true)
                                                                    (probabilistic  
                                                                                    0.25  (is-in visualobject2 place_4_a)
                                                                                    0.5  (is-in visualobject2 place_5_a)
                                                                                    0.25  (is-in visualobject2 place_2_a)
                                                                    )
                                                              )
                                                      0.2000  (ex-in-room cornflakes room__k_a false)
                                      )
                                      (probabilistic  0.9000  (and  (ex-in-room table room__k_a true)
                                                                    (probabilistic  
                                                                                    0.25  (is-in visualobject1 place_1_a)
                                                                                    0.5  (is-in visualobject1 place_2_a)
                                                                                    0.25  (is-in visualobject1 place_3_a)
                                                                    )
                                                              )
                                                      0.1000  (ex-in-room table room__k_a false)
                                      )
                                )
                        0.1000  (and  )
        )
)

(:goal  (and  ))
(:metric maximize (reward ))

)
