for i in {1..5}
do
    ./2048 --total=100000 --block=10000 --limit=10000 --play="load=5x4.bin save=5x4.bin alpha=0.08"
    # ./2048 --total=200000 --block=10000 --limit=10000 --play="load=5x4.bin save=5x4.bin alpha=0.025"
    # ./2048 --total=200000 --block=10000 --limit=10000 --play="load=5x4.bin save=5x4.bin alpha=0.02"
    # ./2048 --total=200000 --block=10000 --limit=10000 --play="load=5x4.bin save=5x4.bin alpha=0.0125"
    # ./2048 --total=200000 --block=10000 --limit=10000 --play="load=5x4.bin save=5x4.bin alpha=0.01"
done