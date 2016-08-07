;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; (lisp2gnx)
;;;		Reads s-expressions from standard input
;;;		Writes GingerXML to standard output
;;;		Error reported - undecided as yet
;;;
;;;	The implementation uses an intermediate form, the GNX-tree. This has the
;;; following shape:
;;;		( <element-name> <attributes> <subtree1> <subtree2>... )
;;;		where attributes has the form
;;;			((<key1> <value1>) (<key2> <value2>) ... )
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (lisp2gnx all-args)
	(let 
		((args (cdr all-args)))
		(run 
			(if (null? args) 
				(current-input-port) 
				(open-input-file (car args))))))

(define (run port)
	(let
		((sexp (read port)))
		(if (eof-object? sexp)
			(exit)
			(begin
				(gnx2output (sexp2expr sexp))
				(force-output)
				(run port)))))
				
				
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
	(mnxRender (cadr attr))
	(display "\""))

(define (mnxRender x)
	(cond 
		((string? x) (string-for-each mnxRenderChar x))
		((char? x) (mnxRenderChar x))
		(else (display x))))

(define (mnxRenderChar ch)
	(case ch
		((#\<) (display "&lt;"))
		((#\>) (display "&gt;"))
		((#\&) (display "&amp;"))
		((#\") (display "&quot;"))
		((#\') (display "&apos;"))
		(else 
			(if 
				(and 
					(char<=? #\space ch) 
					(char<? ch #\del) 
				)
				(display ch)
				(begin
					(display "&#")
					(display (char->integer ch))
					(display ";")
				)
			)
		)
	))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; ( sexp2gnx x mode )
;;;		x is an s-expression
;;;		mode is either 'expr or 'pattern
;;;		returns a GNX-tree
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (sexp2gnx sexp mode)
	(if (list? sexp) 
		(list2gnx sexp mode)
		(atomic2gnx sexp mode)))
		
(define (sexp2expr sexp)
	(sexp2gnx sexp 'expr))
	
(define (sexp2pattern sexp)
	(sexp2gnx sexp 'pattern))

;;;	Curried version of sexp2gnx.
(define (sexp2gnx-mode mode)
	(if (eq? mode 'expr)
		sexp2expr
		sexp2pattern))
	
(define (atomic2gnx x mode) 
	(if (symbol? x) 
		(cond
			((eq? x 'absent) 
				'(absent ((value absent))))
			((eq? x 'nil) 
				'(list ((value empty))))
			(else 
				((if (eq? mode 'expr) gnx-id gnx-var) x)))
		(quote-atom x)))
		
(define (list2gnx sexp mode)
	(if (null? sexp) 
		'(list ((value empty)))
		(form2gnx (car sexp) (cdr sexp) mode)))

(define (form2gnx f args mode)
	(cond
		((eq? f 'quote) 
			(quote2gnx (car args)))
		((eq? f 'quasiquote) 
			(sexp2gnx (expand-quasiquote (car args)) mode))
		((eq? f 'begin)
			(gnx-seq (map (sexp2gnx-mode mode) args)))
		((eq? f 'define) 
			(if (not (eq? mode 'expr)) (throw 'define-in-pattern))
			(define2gnx f args))
		((eq? f 'if) 
			`(if () ,@(map sexp2expr args)))
		((eq? f 'let)
			(let2gnx f args))
		((eq? f 'lambda)
			(lambda2gnx f args))
		((eq? f 'for)
			(for2gnx f args))
		((eq? f 'sysapp)
			`(sysapp ,(car args) ,@(map (sexp2gnx-mode mode) (cdr args))))
		((eq? f 'package)
			(package2gnx f args))
		((eq? f 'import)
			(import2gnx f args))
		(else 
			`(app ()
				,(sexp2gnx f 'expr)
				,(gnx-seq (map (sexp2gnx-mode mode) args))))))

;;; ( package URL ( IMPORT* ) EXPR* )
(define (package2gnx f args)
	(let
		(	(url (car args))
			(imports (cadr args))
			(stmnts (cddr args)))
		`(package ((name ,url))
			,@(map sexp2expr imports)
			,@(map sexp2expr stmnts))))

(define (import2gnx f args)
	(let
		(	(name (car args))
			(attrs (cdr args)))
		`(import 
			(	(name ,name)
				,@(flatten (map impattr2gnx attrs))))))
			
(define (flatten list)
	(if (null? list)
		'()
		(append
			(car list)
			(flatten (cdr list)))))

;;;	MUST RETURN A LIST OF key-values PAIRS
(define (impattr2gnx attr)
	(let
		(	(tag (car attr))
			(args (cdr attr)))
		(cond
			((eq? 'match tag)
				(generate-tags 0 tag args))
			((eq? 'alias tag)
				(list attr))
			((eq? 'qualified)
				(list attr))
			((eq? 'protected)
				(list attr))
			((eq? 'into)
				(generate-tags 0 tag args))	
			(else (throw 'bad-import)))))

(define (generate-tags n root-tag args)
	(if (null? args)
		'()
		(let
			(	(tag-value (car args))
				(rest (cdr args)))
			(cons
				(list (glue root-tag n) tag-value)
				(generate-tags (+ n 1) root-tag rest)))))
				
(define (glue root num)
	(string-append (symbol->string root) (number->string num)))
				

;;; ( for QUERY EXPR* )
(define (for2gnx f args)
	`(for () ,(query2gnx (car args)) ,(gnx-seq (map sexp2expr (cdr args)))))

(define (query2gnx q)
	(let (	(op (car q))
			(rest (cdr q)))
		(cond
			((eq? op 'bind) 
				(binding2gnx rest))
			((eq? op 'in)
				(in2gnx op rest))
			((eq? op 'from)
				(from2gnx op rest))
			(else (throw 'bad-sexp)))))

(define (in2gnx f args)
	(let
		( 	(pat (sexp2pattern (car args)))
			(e (sexp2expr (cadr args))))
		`(,f () ,pat ,e)))

(define (from2gnx f args)
	(let
		(	(pat (sexp2pattern (car args)))
			(from-e (sexp2expr (cadr args)))
			(to-e (sexp2expr (caddr args))))
		`(,f () ,pat ,from-e ,to-e)))
		
	
(define (let2gnx f args)
	(let
		(	(bindings (car args))
			(body (cdr args)))
		`(block ()
			,@(map binding2gnx bindings)
			,@(map sexp2expr body))))

(define (lambda2gnx f args)
	(let
		(
			(params (gnx-seq (map gnx-var (car args))))
			(body (gnx-seq (map sexp2expr (cdr args)))))
		(gnx-lambda-fn params body)))

(define (binding2gnx b)
	`(bind () ,(sexp2pattern (car b)) ,(sexp2expr (cadr b))))

;;; args = ( (f params...) body... )
(define (define2gnx _ args)
	(if (pair? args)
		(if (pair? (car args))
			(let*
				(	(f (caar args))
					(gf (gnx-var f))
					(params (gnx-seq (map gnx-var (cdar args))))
					(body (gnx-seq (map sexp2expr (cdr args)))))
				`(bind () ,gf (fn ((name ,f)) ,params ,body)))
			(let
				(	(var (gnx-var (car args)))
					(value (sexp2expr (cadr args))))
				`(bind () ,var ,value)))
		(throw 'basexp (cons 'define args))))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;	Quoted
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;	mode is passed for consistency, but is not required.
(define (quote2gnx sexp)
	(if (pair? sexp)
		(quote-sexp sexp)
		(quote-atom sexp)))
	
(define (quote-sexp sexp)
	(if (pair? sexp)
		`(sysapp ((name newPair)) ,(quote-sexp (car sexp)) ,(quote-sexp (cdr sexp)))
		(quote-atom sexp)))
		
(define (quote-atom sexp)
	(let ((val `(value ,sexp)))
		(cond
			((number? sexp) `(constant ((type int) ,val)))
			((string? sexp) `(constant ((type string) ,val)))
			((boolean? sexp) `(constant ((type bool) ,val)))
			((char? sexp) `(constant ((type char) ,val)))
			((symbol? sexp) `(constant ((type symbol) ,val)))
			((null? sexp) '(constant ((type list) (value empty))))
			(else (throw 'badsexp sexp)))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;	Quasiquoting
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
(define (expand-quasiquote x)
	(if (pair? x)
		(cond 
			((eq? 'unquote (car x)) 
				(cadr x))
			((and (pair? (car x)) (eq? 'unquote-splicing (caar x))) 
				(list 
					'sysapp 
					'((name append)) 
					(cadar x) 
					(expand-quasiquote (cdr x))))
			(else 
				(list 
					'sysapp
					'((name newPair))
                	(expand-quasiquote (car x))
                	(expand-quasiquote (cdr x)))))
      (list 'quote x)))
      
      
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;	Utility routines for constructing GNX fragments
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
(define (gnx-seq list)
	(if (and (pair? list) (null? (cdr list)))
		(car list)
		`(seq () ,@list)))	

(define (gnx-cons x y)
	`(sysapp ((name newPair)) ,x ,y))

(define (gnx-var name)
	`(var ((name ,name))))
	
(define (gnx-id name)
	`(id ((name ,name))))
	
(define (gnx-lambda-fn args body)
	`(fn () ,args ,body))