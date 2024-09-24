package main

import (
	"fmt"
	"log"
	"os"
	"os/exec"
	"strings"
	"sync"
)

var failed = false

func main() {
	files := getFiles()
	println("running tests...")
	println("-------------")
	resChan := make(chan string)
	var wg sync.WaitGroup
	for _, f := range files {
		wg.Add(1)
		go func(f string) {
			defer wg.Done()
			resBuffer := ""
			res := run(f, &resBuffer)
			if getRes(res, f, &resBuffer) {
				resBuffer = fmt.Sprintf("%s\033[32msuccess:\033[0m test passed in %s\n", resBuffer, f)
			}
			resBuffer = fmt.Sprintf("%s%s\n", resBuffer, "-------------")
			resChan <- resBuffer
		}(f)
	}
	go func() {
		wg.Wait()
		close(resChan)
	}()
	for res := range resChan {
		print(res)
	}
	if failed {
		os.Exit(1)
	}
}

func getFiles() []string {
	files := make([]string, 0)
	f, err := os.ReadDir("./")
	if err != nil {
		log.Fatal(err)
	}

	for _, file := range f {
		if !file.IsDir() {
			dot := strings.Split(file.Name(), ".")
			if len(dot) < 2 {
				continue
			}
			if dot[1] != "ghoul" {
				continue
			}
			files = append(files, file.Name())
			continue
		}
		subFiles, err := os.ReadDir(fmt.Sprintf("./%s", file.Name()))
		if err != nil {
			log.Fatal(err)
		}

		for _, subFile := range subFiles {
			dot := strings.Split(subFile.Name(), ".")
			if dot[1] != "ghoul" {
				continue
			}
			files = append(files, fmt.Sprintf("%s/%s", file.Name(), subFile.Name()))
		}
	}
	return files
}

func run(filepath string, resBuffer *string) string {
	path, err := exec.LookPath("./ghoul")
	if err != nil {
		log.Fatal(err)
	}
	cmd := exec.Command(path, filepath)
	out, err := cmd.CombinedOutput()
	if err != nil {
		failed = true
		*resBuffer = fmt.Sprintf("%s\033[31merror:\033[0m non zero exit, code %s;\n %s\n", *resBuffer, err, out)
		return ""
	}
	outstring := string(out)
	return outstring
}

func getRes(fileData string, fileName string, resBuffer *string) bool {
	data := strings.Split(fileData, "\n")
	data = data[:len(data)-1]
	expected := make([]string, 0)
	actual := make([]string, 0)
	expectedFound := false
	actualFound := false
	for _, line := range data {
		if line == "$expect$" {
			expectedFound = true
			continue
		}
		if line != "$expect$" && !expectedFound {
			continue
		}
		if line == "$actual$" {
			actualFound = true
			continue
		}
		if actualFound && expectedFound {
			actual = append(actual, line)
			continue
		}
		if expectedFound && !actualFound {
			expected = append(expected, line)
			continue
		}
	}
	if !expectedFound {
		*resBuffer = fmt.Sprintf("%s\033[31mtest failed:\033[0m no $expect$ in %s\n", *resBuffer, fileName)
		return false
	}
	if !actualFound {
		*resBuffer = fmt.Sprintf("%s\033[31mtest failed:\033[0m no $actual$ in %s\n", *resBuffer, fileName)
		return false
	}

	if len(expected) != len(actual) {
		*resBuffer = fmt.Sprintf("%s\033[31mtest failed:\033[0m expected length is not equal to actual length in %s; expected is %d, actual is %d\n", *resBuffer, fileName, len(expected), len(actual))
		return false
	}

	anyWrong := false
	for i := range expected {
		if expected[i] != actual[i] {
			*resBuffer = fmt.Sprintf("%s\033[31mtest failed:\033[0m test failled in %s, on comparison %d; expected: \033[32m%s\033[0m; but got: \033[31m%s\033[0m\n", *resBuffer, fileName, i+1, expected[i], actual[i])
			anyWrong = true
		}
	}
	if anyWrong {
		failed = true
		return false
	}

	return true
}
