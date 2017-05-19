﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Numerics;

namespace Square {
    public static class RNG {
        private static Random RandCtx = new Random();

        public static int Next() {
            lock(RandCtx) {
                return RandCtx.Next();
            }
        }

        public static int Next(int maxValue) {
            lock(RandCtx) {
                return RandCtx.Next(maxValue);
            }
        }

        public static int Next(int minValue, int maxValue) {
            lock(RandCtx) {
                return RandCtx.Next(minValue, maxValue);
            }
        }

        public static double NextDouble() {
            lock(RandCtx) {
                return RandCtx.NextDouble();
            }
        }

        public static void NextBytes(byte[] buffer) {
            lock(RandCtx) {
                RandCtx.NextBytes(buffer);
            }
        }

        public static byte[] NextBytes(int length) {
            lock(RandCtx) {
                var buffer = new byte[length];
                RandCtx.NextBytes(buffer);
                return buffer;
            }
        }

        public static BigInteger NextPrime(int byteCount) {
            var bytes = new byte[byteCount];
            BigInteger prime;

            do {
                NextBytes(bytes);
                prime = BigInteger.Abs(new BigInteger(bytes)) | 1;
            } while(!prime.IsProbablePrime());

            return prime;
        }

        public static BigInteger NextBigInt(BigInteger minValue, BigInteger maxValue) {
            var byteCount = maxValue.ToByteArray().Length;
            var randomNumber = BigInteger.Abs(new BigInteger(NextBytes(byteCount)));

            var delta = maxValue - minValue + 1;
            return minValue + (randomNumber % delta);
        }

        public static BigInteger NextBigInt(int byteCount) {
            return BigInteger.Abs(new BigInteger(NextBytes(byteCount)));
        }
    }
}
