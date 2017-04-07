;; => 48
(* (+ 2 2) (/ (* (+ 3 5) (/ 30 10)) 2))

;; => 3
(sqrt 9)

;; => 3^2 = 9
(expt 3 2)

;; => 2.302585092994046
(log 10)

;; => -0.26237485370392877
(sin 50)

;; area of circle
(define (area-of-circle r)
  (* 3.14 (expt r 2)))

;; => 78.5
(area-of-circle 5)

;; cal area of ring
(define (area-of-ring outter-c inner-c)
  (- (area-of-circle outter-c)
     (area-of-circle inner-c)))

;; => 235.5   
(area-of-ring 10 5)

(define (is-5? x)
  (= x 5))

;; => #t
(is-5? 5)

(define (is-between-5-10? x)
  (and (< 5 x) (< x 10)))

;; => #t
(is-between-5-10? 6)


(define n 12)

;; => 0.5
(cond 
      ((<= n 10) 0.1)
      ((<= n 50) 0.5)
      ((<= n 90) 0.9)
      (else 1))
    
(set! n 80)

;; => 0.9
(cond 
      ((<= n 10) 0.1)
      ((<= n 50) 0.5)
      ((<= n 90) 0.9)
      (else 1))
    
(set! n 97)

;; => 1
(cond 
      ((<= n 10) 0.1)
      ((<= n 50) 0.5)
      ((<= n 90) 0.9)
      (else 1))
    
    
(define (interast-rate amount)
  (cond 
        ((<= amount 1000) 0.040)
        ((<= amount 5000) 0.045)
        ((> amount 5000) 0.050)))
      
;; => 0.045
(interast-rate 5000)

(set! n 100)

;; => none 
(cond 
      ((<= n 10) 0.1)
      ((<= n 50) 0.5)
      ((<= n 90) 0.9)
      )

