import h5py
import argparse

parser = argparse.ArgumentParser(description='Plot ECG and QRS data.')
parser.add_argument("input", help="ECG file", type=str)
parser.add_argument("out", help="HDF5 file", type=str)
args = parser.parse_args()

ecgValue = []
qrsValue = []

with open(args.input, 'r') as srcFile:
    print("Reading of QRS/ECG file. It may take some time.")
    for line in srcFile:
        date, index, sample, qrs, ecg, aFibN, aFibF = line.split("\t")
        ecgValue.append(float(ecg))
        qrsValue.append(float(qrs))
    print("QRS/ECG data readed successfully!")
srcFile.close()

print("Createing HDF5 file at: ", args.out)
f = h5py.File(args.out, 'w')
f.create_dataset('ecg', data=ecgValue)
f.create_dataset('qrs', data=qrsValue)
f.close()