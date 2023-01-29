# TP2 de Kerbourc'h Sébastien

`pandoc -s --toc tp2.md --css=./github-pandoc.css -o tp1.html`

## lscpu

```
Architecture :                          x86_64
Mode(s) opératoire(s) des processeurs : 32-bit, 64-bit
Boutisme :                              Little Endian
Address sizes:                          39 bits physical, 48 bits virtual
Processeur(s) :                         16
Liste de processeur(s) en ligne :       0-15
Thread(s) par cœur :                    2
Cœur(s) par socket :                    8
Socket(s) :                             1
Nœud(s) NUMA :                          1
Identifiant constructeur :              GenuineIntel
Famille de processeur :                 6
Modèle :                                141
Nom de modèle :                         11th Gen Intel(R) Core(TM) i7-11800H @ 2.30GHz
Révision :                              1
Vitesse du processeur en MHz :          2300.000
Vitesse maximale du processeur en MHz : 4600,0000
Vitesse minimale du processeur en MHz : 800,0000
BogoMIPS :                              4608.00
Virtualisation :                        VT-x
Cache L1d :                             384 KiB
Cache L1i :                             256 KiB
Cache L2 :                              10 MiB
Cache L3 :                              24 MiB
Nœud NUMA 0 de processeur(s) :          0-15
Vulnerability Itlb multihit:            Not affected
Vulnerability L1tf:                     Not affected
Vulnerability Mds:                      Not affected
Vulnerability Meltdown:                 Not affected
Vulnerability Mmio stale data:          Not affected
Vulnerability Retbleed:                 Not affected
Vulnerability Spec store bypass:        Mitigation; Speculative Store Bypass disabled via prctl and seccomp
Vulnerability Spectre v1:               Mitigation; usercopy/swapgs barriers and __user pointer sanitization
Vulnerability Spectre v2:               Mitigation; Enhanced IBRS, IBPB conditional, RSB filling, PBRSB-eIBRS SW sequence
Vulnerability Srbds:                    Not affected
Vulnerability Tsx async abort:          Not affected
Drapaux :                               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr
                                         sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs bts rep_go
                                        od nopl xtopology nonstop_tsc cpuid aperfmperf tsc_known_freq pni pclmulqdq dtes64 monitor ds_cpl
                                         vmx est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_ti
                                        mer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch cpuid_fault epb cat_l2 invpcid_single cdp
                                        _l2 ssbd ibrs ibpb stibp ibrs_enhanced tpr_shadow vnmi flexpriority ept vpid ept_ad fsgsbase tsc_
                                        adjust bmi1 avx2 smep bmi2 erms invpcid rdt_a avx512f avx512dq rdseed adx smap avx512ifma clflush
                                        opt clwb intel_pt avx512cd sha_ni avx512bw avx512vl xsaveopt xsavec xgetbv1 xsaves split_lock_det
                                        ect dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp hwp_pkg_req avx512vbmi umip pku
                                         ospke avx512_vbmi2 gfni vaes vpclmulqdq avx512_vnni avx512_bitalg avx512_vpopcntdq rdpid movdiri
                                         movdir64b fsrm avx512_vp2intersect md_clear flush_l1d arch_capabilities
```

## Produit matrice-matrice

### Permutation des boucles

_Expliquer comment est compilé le code (ligne de make ou de gcc) : on aura besoin de savoir l'optim, les paramètres, etc. Par exemple :_

`make TestProduct.exe && ./TestProduct.exe 1024`

| ordre           | time    | MFlops (n=1024) | MFlops(n=2048) |
| --------------- | ------- | --------------- | -------------- |
| i,j,k (origine) | 2.73764 | 782.476         |
| j,i,k           | 2.22903 | 963.417         | 830.926        |
| i,k,j           | 3.91577 | 548.419         |
| k,i,j           | 3.87989 | 553.491         |
| j,k,i           | 0.46055 | 4662.81         |
| k,j,i           | 0.47963 | 4477.32         | 2819           |

_Discussion des résultats_

Dans la mémoire les données sont enregistrées "par i". Par exemple :

| Stockage |           |           |           | ... |              |
| -------- | --------- | --------- | --------- | --- | ------------ |
| Mémoire  | case 0, 0 | case 1, 0 | case 2, 0 | ... | case 1023, 1 |
| j        | 0         | 0         | 0         | ... | 1            |
| i        | 0         | 1         | 2         | ... | 0            |

Dans le cas où la boucle i est en dernière, on va pouvoir charger en cache plusieurs case de la matrice et ces cases pourront être utilisées
dans la suite de la boucle. Si ce n'est pas le cas, on va devoir recharger le cache plus de fois depuis la ram, ce qui va ralentir le temps d'execution.

### OMP sur la meilleure boucle

`make TestProduct.exe && OMP_NUM_THREADS=8 ./TestProduct.exe 1024`

| OMP_NUM | MFlops(n=1024) | MFlops(n=2048) | MFlops(n=512) | MFlops(n=4096) |
| ------- | -------------- | -------------- | ------------- | -------------- |
| 1       | 4634           | 3308.8         | 4071.39       | 3417.74        |
| 2       | 9055.91        | 6143.01        | 9154.55       | 6257.03        |
| 3       | 13553.2        | 8875.71        | 13333.7       | 8321.8         |
| 4       | 16997.3        | 10712.8        | 17358.1       | 10836.4        |
| 5       | 19379          | 11774.6        | 19521.7       | 12938.8        |
| 6       | 21107.7        | 13438.8        | 22201.8       | 13626.4        |
| 7       | 22213.5        | 13681.8        | 22207.4       | 14175.9        |
| 8       | 24420.2        | 14039.5        | 24670.9       | 14700.9        |

### Produit par blocs

`make TestProduct.exe && ./TestProduct.exe 1024`

| szBlock        | MFlops(n=1024) | MFlops(n=2048) | MFlops(n=512) | MFlops(n=4096) |
| -------------- | -------------- | -------------- | ------------- | -------------- |
| origine (=max) |
| 32             | 3411.21        | 3464.52        | 3391.53       | 3477.14        |
| 64             | 3813.37        | 3848.41        | 4045.21       | 3493.26        |
| 128            | 4090.52        | 4156.15        | 4263.22       | 3719.73        |
| 256            | 4089.53        | 3914.87        | 3904.8        | 3750.99        |
| 512            | 3820.16        | 3685.72        | 4481.6        | 3768.58        |
| 1024           | 4378.03        | 4275.69        | 4618.72       | 4087.35        |

TODO 1.6

### Bloc + OMP

| szBlock         | OMP_NUM   | MFlops(n=1024) | MFlops(n=2048)                                    | MFlops(n=512) | MFlops(n=4096) |
| --------------- | --------- | -------------- | ------------------------------------------------- | ------------- | -------------- |
| A.nbCols        | 1         | 4415.11        | 3191.81                                           | 4585.89       | 3296.53        |
| 512             | 8         | 8375.1         | 16496.4                                           | 4477.86       | 25906.7        |
| --------------- | --------- | -------------- | ------------------------------------------------- | ------------- | -------------- |
| Speed-up        |           | 1,896917631    | 5,168352753                                       | 0,976528135   | 7,858778776    |
| --------------- | --------- | -------------- | ------------------------------------------------- | ------------- | -------------- |

On peut remarquer que la version scalaire parallélisée est meilleure pour les matrices proches de la taille des blocks. A contrario,
pour de grosses matrices la version par blocs parallélisée est meilleure.

TODO 1.7

### Comparaison with BLAS

# Tips

```
	env
	OMP_NUM_THREADS=4 ./produitMatriceMatrice.exe
```

Resultats obtenus

| MFlops(n=1024) | MFlops(n=2048) | MFlops(n=512) |
| -------------- | -------------- | ------------- |
| 5079.65        | 3301.85        | 5092.96       |

J'ai l'impression que le produit par block de 512 parallélisé est meilleur.

```
    $ for i in $(seq 1 4); do elap=$(OMP_NUM_THREADS=$i ./TestProductOmp.exe|grep "Temps CPU"|cut -d " " -f 7); echo -e "$i\t$elap"; done > timers.out
```
