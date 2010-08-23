;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; ( gnx2output g-tree )
;;;		g-tree is a GNX-tree
;;;		result is unused
;;;		effect is to print the GNX-tree in GingerXML format on standard output
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



(define (gnx2output x)
	(gnx2out x)
	(newline))

(define (gnx2out x)
	(let
		(	
			(element-name (car x))
			(attributes (cadr x))
			(children (cddr x))
		)
		(display "<")
		(display element-name)
		(map attr2out attributes)
		(if (null? children)
			(display "/>")
			(begin
				(display ">")
				(map gnx2out children)
				(display "</")
				(display element-name)
				(display ">")))))
				
(define (attr2out attr)
	(display " ")
	(display (car attr))
	(display "=\"")
	(display (cadr attr))
	(display "\""))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; ( sexp2gnx x )
;;;		x is an s-expression
;;;		returns a GNX-tree
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (sexp2gnx sexp)
	(if (list? sexp) 
		(list2gnx sexp)
		(atomic2gnx sexp)))

	
(define (atomic2gnx x) 
	(if (symbol? x) 
		(cond
			((eq? x 'absent) 
				'(absent ((value absent))))
			((eq? x 'nil) 
				'(list ((value empty))))
			(else 
				`(id ((name ,x)))))
		(quote-atom x)))
		
(define (quote-atom sexp)
	(let ((val `((value ,sexp))))
		(cond
			((number? sexp) `(int ,val))
			((string? sexp) `(string ,val))
			((boolean? sexp) `(bool ,val))
			((char? sexp) `(char ,val))
			((symbol? sexp) `(symbol ,val))
			((null? sexp) '(list ((value empty))))
			(else (throw 'badsexp sexp)))))

(define (list2gnx sexp)
	(if (null? sexp) 
		'(list ((value empty)))
		(form2gnx (car sexp) (cdr sexp))))

(define (form2gnx f args)
	(cond
		((eq? f 'quote) 
			(quote2gnx (car args)))
		((eq? f 'quasiquote) 
			(sexp2gnx (expand-quasiquote (car args))))
		((eq? f 'begin)
			(gnx-seq (map sexp2gnx args)))
		((eq? f 'define) 
			(define2gnx f args))
		((eq? f 'if) 
			`(if () ,@(map sexp2gnx args)))
		(else 
			`(app ()
				,(sexp2gnx f)
				,(gnx-seq (map sexp2gnx args))))))

(define (quote2gnx sexp)
	(if (pair? sexp)
		(quote-sexp sexp)
		(quote-atom sexp)))
	
(define (quote-sexp sexp)
	(if (pair? sexp)
		`(sysapp ((name newPair)) ,(quote-sexp (car sexp)) ,(quote-sexp (cdr sexp)))
		(quote-atom sexp)))
	
(define (expand-quasiquote x)
	(if (pair? x)
		(cond 
			((eq? 'unquote (car x)) 
				(cadr x))
			((and (pair? (car x)) (eq? 'unquote-splicing (caar x))) 
				(list 'append (cadar x) (expand-quasiquote (cdr x))))
			(else 
				(list 
					'cons 
                	(expand-quasiquote (car x))
                	(expand-quasiquote (cdr x)))))
      (list 'quote x)))
	
(define (gnx-seq list)
	(if (and (pair? list) (null? (cdr list)))
		(car list)
		`(seq () ,@list)))	

(define (gnx-cons x y)
	`(sysapp ((name newPair)) ,x ,y))
	
