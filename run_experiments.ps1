$SIZES = @(1000, 5000, 10000, 50000, 100000)
$THREADS = @(1, 2, 4, 8)
$REPS = 3

"size,type,threads,time,overhead" | Out-File -FilePath "results.csv" -Encoding UTF8

foreach ($size in $SIZES) {
    # Serial
    $total = 0.0
    $success_count = 0
    foreach ($i in 1..$REPS) {
        try {
            $output = .\odd_even_serial.exe $size 2>&1 | Out-String
            
            # Extração robusta de tempo
            if ($output -match 'SERIAL_TIME: ([\d.]+)') {
                $time_val = [double]$matches[1]
                $total += $time_val
                $success_count++
                Write-Host "Serial $size - rep $i : $time_val s"
            }
            else {
                Write-Host "Serial: Time not found for size=$size. Rep $i"
                Write-Host "Output: $output"
            }
        }
        catch {
            Write-Host "Serial error: $_"
        }
    }
    if ($success_count -gt 0) {
        $avg = $total / $success_count
        "$size,serial,1,$avg,0" | Out-File -FilePath "results.csv" -Append -Encoding UTF8
    }
    else {
        "$size,serial,1,-1,0" | Out-File -FilePath "results.csv" -Append -Encoding UTF8
    }

    # OpenMP
    foreach ($t in $THREADS) {
        $total = 0.0
        $success_count = 0
        foreach ($i in 1..$REPS) {
            try {
                $output = .\odd_even_openmp.exe $size $t 2>&1 | Out-String
                
                # Extração robusta de tempo
                if ($output -match 'OPENMP_TIME: ([\d.]+)') {
                    $time_val = [double]$matches[1]
                    $total += $time_val
                    $success_count++
                    Write-Host "OpenMP $size - $t threads - rep $i : $time_val s"
                }
                else {
                    Write-Host "OpenMP: Time not found for size=$size, threads=$t. Rep $i"
                    Write-Host "Output: $output"
                }
            }
            catch {
                Write-Host "OpenMP error: $_"
            }
        }
        if ($success_count -gt 0) {
            $avg = $total / $success_count
            "$size,openmp,$t,$avg,0" | Out-File -FilePath "results.csv" -Append -Encoding UTF8
        }
        else {
            "$size,openmp,$t,-1,0" | Out-File -FilePath "results.csv" -Append -Encoding UTF8
        }
    }

    # MPI
    $MPI_BIN = "C:\Program Files\Microsoft MPI\Bin\mpiexec.exe"
    foreach ($p in $THREADS) {
        $total_time = 0.0
        $total_overhead = 0.0
        $success_count = 0
        
        foreach ($i in 1..$REPS) {
            try {
                $output = & $MPI_BIN -n $p .\odd_even_mpi.exe $size 2>&1 | Out-String
                
                if ($LASTEXITCODE -eq 0) {
                    # Extração robusta de tempo e overhead
                    $time_line = $output | Select-String -Pattern 'MPI_TOTAL_TIME: ([\d.]+)'
                    $overhead_line = $output | Select-String -Pattern 'MPI_OVERHEAD: ([\d.]+)'
                    
                    if ($time_line -and $overhead_line) {
                        $time_val = [double]$time_line.Matches.Groups[1].Value
                        $overhead_val = [double]$overhead_line.Matches.Groups[1].Value
                        
                        $total_time += $time_val
                        $total_overhead += $overhead_val
                        $success_count++
                        Write-Host "MPI $size - $p processes - rep $i : $time_val s ($overhead_val% overhead)"
                    }
                    else {
                        Write-Host "MPI: Time/overhead not found for size=$size, processes=$p. Rep $i"
                        Write-Host "Output: $output"
                    }
                }
                else {
                    Write-Host "MPI: Error $LASTEXITCODE for size=$size, processes=$p. Rep $i"
                    Write-Host "Output: $output"
                }
            }
            catch {
                Write-Host "MPI exception: $_"
            }
        }
        
        if ($success_count -gt 0) {
            $avg_time = $total_time / $success_count
            $avg_overhead = $total_overhead / $success_count
            "$size,mpi,$p,$avg_time,$avg_overhead" | Out-File -FilePath "results.csv" -Append -Encoding UTF8
        }
        else {
            "$size,mpi,$p,-1,-1" | Out-File -FilePath "results.csv" -Append -Encoding UTF8
        }
    }
}