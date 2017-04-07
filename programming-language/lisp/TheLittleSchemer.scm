(define atom?
  (lambda (x)
    (and (not (pair? x)) (not (null? x)))
    ))
  
  ;;atom? test  -> #t
  (display 
           (atom? (quote MathxH)))
  (newline)
  
  ; this is equal to quote    -> #t
  (display 
           (atom? 'MathxH))
  (newline)
  
  ; -> #t
  (display 
           (atom? #t))
  (newline)
  
  ; -> #t
  (display 
           (atom? #f))
  (newline)
  
  ; -> #t
   (display 
           (atom? "MathxH"))
  (newline)
  
  ; -> #t
  (display 
           (atom? 1992))
  (newline)
  
  ; -> #t
   (display 
           (atom? (atom?)))
  (newline)
  
  ; -> #f
  (display 
           (atom? ()))
  (newline)
  
   ; -> #t
  (display 
           (atom? "*%@#$abc"))
  (newline)
  
  ;; Hello World   -> Hello World
  (display "Hello World")
  (newline)
  
