test_dir=$(dirname ${BASH_SOURCE[0]})
${test_dir}/../decode_reencode ${test_dir}/../MIDI_files/sample_UNkn_chunks.mid ${test_dir}/encoded_files/sample_UNkn_chunks.mid > ${test_dir}/results/sample_UNkn_chunks.txt

result=$(head -n 1 ${test_dir}/results/sample_UNkn_chunks.txt)

if [ "$result" = "complete" ]; then
    echo "pass"
else
    echo "fail"
fi

