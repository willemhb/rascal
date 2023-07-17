;; macro implementation
(def *syntax-table* (table))

(mac catch
  (((cause) & handler) & body)
  (mac throw (cause) `(perform :exception ~cause))
  `(handle (#op #cause & #args)))