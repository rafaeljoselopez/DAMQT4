DATE=$(date +%Y%m%d)
git archive \
    --format=tar.gz \
    --prefix="DAMQT4-${DATE}/" \
    -o "../DAMQT4-${DATE}.tar.gz" \
    HEAD
