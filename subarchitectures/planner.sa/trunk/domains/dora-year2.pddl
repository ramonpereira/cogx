(define (domain dora-test-100)
  (:requirements :mapl :adl :fluents :durative-actions :partial-observability :dynamic-objects)

  (:types
   cone place room - object
   virtual-place - place
   visualobject - movable
   robot - agent
   robot - movable
   place_status label category - object
   )

  (:predicates
   (connected ?p1 ?p2 - place)
   ;; derived predicates
   (cones-exist ?l - label ?r - room)
   (obj-possibly-in-room ?o - visualobject ?r - room)
   (fully_explored ?r)

   ;;virtual predicates
   (cones_created ?l - label ?r - room)
   (started)
   (done)
   )

  (:functions
   (is-in ?o - movable) - place
   (is-in ?c - cone) - place
   (in-room ?p - place) - room
   (in-room ?c - cone) - room
   (category ?r - room) - category
   (label ?o - visualobject) - label
   (label ?c - cone) - label
;   (cone-label ?c - cone) - label
   (ex-in-room ?l - label ?r - room) - boolean
   (probability ?c - cone) - number
   (p-is-in ?p - place) - number
   (dora__in ?l - label ?c - category ) - number
   (p-category ?r - room ?c - category ) - number
   (total-p-costs) - number
   )

  (:constants
   dummy-cone - label
   kitchen office living_room - category
   ;;cornflakes table mug - label ;;oven fridge book board-game - label
   )

  (:init-rule init
              :effect (and (assign (total-p-costs) 200))
              )

  (:init-rule objects
              :parameters(?l - label)
              :precondition (not (exists (?o - visualobject)
                                      (= (label ?o) ?l)))
              :effect (create (?o - visualobject) (and
                                                   (assign (label ?o) ?l)
                                                   (assign (is-in ?o) UNKNOWN)))
              )

  (:init-rule categories
              :parameters(?r - room)
              :effect (assign-probabilistic (category ?r) 
                                            0.3 kitchen
                                            0.3 office
                                            0.3 living_room)
              )

  (:init-rule virtual-places
              :parameters(?r - room)
              :precondition (not (exists (?p - virtual-place)
                                         (= (in-room ?p) ?r)))
              :effect (and (create (?p - virtual-place) (and
                                                         (assign (in-room ?p) ?r)))
                           )
              )

  (:derived (obj-possibly-in-room ?o - visualobject ?r - room)
            (exists (?p - place) (and (= (in-room ?p) ?r)
                                      (in-domain (is-in ?o) ?p))))

  (:derived (cones-exist ?l - label ?r - room)
            (exists (?c - cone ?p - place) (and (= (in-room ?p) ?r)
                                                (= (is-in ?c) ?p)
                                                (= (label ?c) ?l)))
            )

  ;; (:derived (fully_explored ?r - room)
  ;;           (forall (?p - place) (or (not (= (in-room ?p) ?r))
  ;;                                    (

  (:action sample_existence
           :agent (?a - agent)
           :parameters (?l - label ?r - room ?c - category)
           :precondition (= (category ?r) ?c)
           :effect (probabilistic (dora__in ?l ?c) (assign (ex-in-room ?l ?r) true))
                                                   ;;(assign (ex-in-room ?l ?r) false))
           )

  (:action sample_is_in
           :agent (?a - agent)
           :parameters (?l - label ?r - room ?p - place ?c - cone ?o - visualobject)
           :precondition (and (= (in-room ?p) ?r)
                              (= (is-in ?c) ?p)
                              (= (label ?c) ?l)
                              (= (label ?o) ?l)
                              (in-domain (is-in ?o) ?p) 
                              (= (ex-in-room ?l ?r) true))
           :effect (probabilistic (probability ?c) (assign (is-in ?o) ?p))
           )

  (:action sample_is_in_virtual
           :agent (?a - agent)
           :parameters (?l - label ?r - room ?p - virtual-place ?o - visualobject)
           :precondition (and (= (in-room ?p) ?r)
                              (= (label ?o) ?l)
                              (obj-possibly-in-room ?o ?r)
                              (not (cones-exist ?l ?r))
                              (= (ex-in-room ?l ?r) true))
           :effect (probabilistic 0.9 (assign (is-in ?o) ?p))
           )

   ;; (:durative-action spin
   ;;                   :agent (?a - robot)
   ;;                   :duration (= ?duration 0)
   ;;                   :condition (over all (done))
   ;;                   :effect (and)
   ;;                   )

   (:durative-action move
                     :agent (?a - robot)
                     :parameters (?to - place)
                     :variables (?from - place)
                     :duration (= ?duration 5)
                     :condition (and (over all (or (connected ?from ?to)
                                                   (connected ?to ?from)
                                                   ))
                                     (over all (not (done)))
                                     (at start (= (is-in ?a) ?from)))
                     :effect (and (change (is-in ?a) ?to)
                                  (at start (started)))
                     )

   (:durative-action create_cones
                     :agent (?a - robot)
                     :parameters (?l - label ?r - room)
                     :variables (?p - place)
                     :duration (= ?duration 10)
                     :condition (and (over all (and (= (is-in ?a) ?p)
                                                    (= (in-room ?p) ?r)
                                                    ;;(hyp (ex-in-room ?l ?r) true)
                                                    (not (done))))
                                     )
                     :effect (and (at end (cones_created ?l ?r))
                                  (at start (started)))
                     )


   ;; (:durative-action process_cone
   ;;                   :agent (?a - robot)
   ;;                   :parameters (?c - cone)
   ;;                   :variables (?o - visualobject ?l - label ?p - place)
   ;;                   :duration (= ?duration 4)
   ;;                   :condition (over all (and (not (done))
   ;;                                             (= (is-in ?a) ?p)
   ;;                                             (= (is-in ?c) ?p)
   ;;                                             (= (label ?c) ?l)
   ;;                                             (= (label ?o) ?l)))
   ;;                   :effect (and (at start (started)))
   ;;                   )

; nah: execution is expecting this:
;   (:durative-action process_all_cones_at_place
;                     :agent (?a - robot)
;                     :parameters (?p - place ?l - label)



   (:durative-action process_virtual_place
                     :agent (?a - robot)
                     :parameters (?o - visualobject ?l - label ?r - room ?p - virtual-place)
                     :duration (= ?duration 1)
                     :condition (over all (and (not (done))
                                               (cones_created ?l ?r)
                                               (= (in-room ?p) ?r)
                                               (= (label ?o) ?l)));(over all (= (is-in ?a) ?c))
                                     ;(at start (hyp (is-in ?o) ?c)))
                     :effect (and ;;(at end (assign (really-is-in ?o) ?c))
                                  ;;(at end (kval ?a (is-in ?o)))
                                  (at start (started)))
                     )

   (:observe visual_object_virtual
             :agent (?a - robot)
             :parameters (?o - visualobject ?l - label ?r - room ?p - virtual-place)
             :execution (process_virtual_place ?a ?o ?l ?r ?p)
             :effect (when (= (is-in ?o) ?p)
                       (probabilistic 0.8 (observed (is-in ?o) ?p)))
             )
                     
   (:durative-action process_all_cones_at_place
                     :agent (?a - robot)
                     :parameters (?p - place ?l - label )
                     :variables (?o - visualobject)
                     :duration (= ?duration 1)
                     :condition (over all (and (not (done))
                                               (= (is-in ?a) ?p)
                                               (= (label ?o) ?l)))
                                        ;(over all )
                                     ;(at start (hyp (is-in ?o) ?c)))
                     :effect (and ;;(at end (assign (really-is-in ?o) ?c))
                                  ;;(at end (kval ?a (is-in ?o)))
                                  (at start (started)))
                     )

   (:observe visual_object
             :agent (?a - robot)
             :parameters (?o - visualobject ?l - label ?p - place)
             :execution (process_all_cones_at_place ?a ?p ?l ?o)
             :effect (when (= (is-in ?o) ?p)
                       (probabilistic 0.8 (observed (is-in ?o) ?p)))
             )
             
)
