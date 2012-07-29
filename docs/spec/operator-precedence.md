Order precedence
================

This is currently taken from PHP as an example.

<table>
<tr><td>non-associative     </td><td>clone new                                                     </td><td>clone and new</td></tr>
<tr><td>left                </td><td>[                                                             </td><td>array()</td></tr>
<tr><td>right               </td><td>++ -- ~ (int) (float) (string) (array) (object) (bool) @      </td><td>types and increment/decrement</td></tr>
<tr><td>non-associative     </td><td>instanceof                                                    </td><td>types</td></tr>
<tr><td>right               </td><td>!                                                             </td><td>logical</td></tr>
<tr><td>left                </td><td>* / %                                                         </td><td>arithmetic</td></tr>
<tr><td>left                </td><td>+ - .                                                         </td><td>arithmetic and string</td>
<tr><td>left                </td><td><< >>                                                         </td><td>bitwise</td></tr>
<tr><td>non-associative     </td><td>< <= > >=                                                     </td><td>comparison</td></tr>
<tr><td>non-associative     </td><td>== != === !== <>                                              </td><td>comparison</td></tr>
<tr><td>left                </td><td>&                                                             </td><td>bitwise and references</td>
<tr><td>left                </td><td>^                                                             </td><td>bitwise</td></tr>
<tr><td>left                </td><td>|                                                             </td><td>bitwise</td></tr>
<tr><td>left                </td><td>&&                                                            </td><td>logical</td></tr>
<tr><td>left                </td><td>||                                                            </td><td>logical</td></tr>
<tr><td>left                </td><td>? :                                                           </td><td>ternary</td></tr>
<tr><td>right               </td><td>= += -= *= /= .= %= &= |= ^= <<= >>= =>                       </td><td>assignment</td></tr>
<tr><td>left                </td><td>and                                                           </td><td>logical</td></tr>
<tr><td>left                </td><td>xor                                                           </td><td>logical</td></tr>
<tr><td>left                </td><td>or                                                            </td><td>logical</td></tr>
<tr><td>left                </td><td>,                                                             </td><td>many uses</td></tr>
</table>