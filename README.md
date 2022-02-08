# lisp
a toy lisp in c++, built around storing `std::string`s in `std::vector`s

# Primitive Operators
```lisp
;; +/-
lisp> (+ 1 2 (- 3 (+ 4) (- (- 5))))
-3
;; QUOTE
lisp> (quote (+ 1 2))
(+ 1 2)
;; ATOM
lisp> (atom ())
t
lisp> (atom (list 1))
()
;; EQ
lisp> (eq (list 1 (+ 2 3)) (list 1 (- 100 95)))
t
;; CAR
lisp> (car (list (list 1 2) 3 4))
(list 1 2 ()) ;; `(list 1 2)` has `()` `cons`'ed to end
;; CDR
lisp> (cdr (list (list 1 2) 3 4))
(list 3 4 ())
;; CONS
lisp> (cons 1 (list 2 3))
(cons 1 (list 2 3))
;; COND
lisp> (cond (list (list () 42) (list t 1)))
1
```
