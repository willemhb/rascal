(def *opcodes* (table :op-no-op         0
                      :op-start         1
                      :op-pop           2
                      :op-closure       3
                      :op-control       4
                      :op-return        5
                      :op-argcount      6
                      :op-var-argcount  7
                      :op-call          8
                      :op-load-value    9
                      :op-load-global  10
                      :op-put-global   11
                      :op-jump         12
                      :op-jump-nil     13
                      :op-load-closure 14
                      :op-put-closure  15))

