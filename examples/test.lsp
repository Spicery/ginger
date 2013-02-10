(define (twice n) (+ n n))
(twice 4)

;; (reduce + 0
;;  (filter
;;    (lambda (n)
;;      (or (= (remainder n 3) 0) (= (remainder n 5) 0)))
;;    (iota 1000)))

;; named let is another common loop
(let loop ((i x))
  (cond ((<= i y)
         ;; i is set to every integer from x to y, step-size 7
         ;; ...
         (loop (+ i 7)))))  ; tail-recursive call

(display "Infancy is: ")
(do ((i 0 (1+ i)))
    ((> i 2))
  (format #t "~A " i))
(newline)

(display "Toddling is: ")
(let ((i 3))
  (while (<= i 4)
         (format #t "~A " i)
         (set! i (1+ i))))
(newline)

(display "Childhood is: ")
(let loop ((i 5))
  (cond ((<= i 12)
         (format #t "~A " i)
         (loop (1+ i)))))
(newline)
